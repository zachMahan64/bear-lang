//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/def_visitor.hpp"
#include "compiler/hir/compt_expr_solver.hpp"
#include "compiler/hir/context.hpp"
#include "compiler/hir/def.hpp"
#include "compiler/hir/diagnostic.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/hir/type.hpp"
#include "compiler/hir/type_resolver.hpp"
#include <cassert>
#include <optional>

namespace hir {

void TopLevelDefVisitor::resolve_top_level_definitions() {
    assert(!this->began_resolution && "already began resolving top level declarations");
    this->began_resolution = true;
    auto last_top_level = context.end_def_id();
    for (auto d = context.begin_def_id(); d != last_top_level; d++) {
        visit_as_dependent(d);
    }
}
DefId TopLevelDefVisitor::visit(DefId def) {
    if (context.resol_state_of(def) == Def::resol_state::resolved) {
        return def;
    }
    def_stack.push_back(def);
    context.set_resol_state_of(def, Def::resol_state::in_progress);
    auto d = resolve_def(def);
    context.set_resol_state_of(def, Def::resol_state::resolved);
    def_stack.pop_back();
    return d;
}

DefId TopLevelDefVisitor::visit_as_dependent(DefId def) {
    if (context.resol_state_of(def) == Def::resol_state::in_progress) {
        // reports the double diagnostic revealing the origin of the circular def
        report_cycle(def);
        // return as to prevent infinite recursion
        return def;
    }
    return visit(def);
}
DefId TopLevelDefVisitor::visit_as_transparent(DefId def) {
    // in_progress is fine here since this is a transparent visitation (pointer or reference, so no
    // information dependency)
    return visit(def);
}

DefId TopLevelDefVisitor::resolve_def(DefId did) {

    auto check_to_err_when_compt_is_not_mut = [&](TypeId tid, const Def& def) {
        if (contains_mut(context, tid) && def.compt) {
            context.emplace_diagnostic(context.type(tid).span,
                                       diag_code::compt_variable_should_be_immutable,
                                       diag_type::error);
        }
    };

    const ast_stmt* stmt = context.def_ast_node(did);
    auto scope = context.containing_scope(did);
    Def& def = context.def(did);
    Span span = def.span;
    //  TODO write handlers
    switch (stmt->type) {
    case AST_STMT_VAR_DECL: {
        OptId<TypeId> maybe_type = TypeResolver<TopLevelDefVisitor>{context, *this}.resolve_type(
            span.file_id, scope, stmt->stmt.var_decl.type);
        if (!maybe_type.has_value()) {
            return did; // maybe set a special value to indicate error differently
        }
        // compt =/= mut guard
        check_to_err_when_compt_is_not_mut(maybe_type.as_id(), def);
        if (def.compt) {
            context.emplace_diagnostic_with_message_value(
                def.span, diag_code::a_compt_variable_should_be_explicitly_initialized,
                diag_type::error, DiagnosticSymbolBeforeMessage{.sid = def.name});
        }
        def.set_value(DefVariable{.type = maybe_type.as_id(),
                                  .name = context.symbol_id(stmt->stmt.var_decl.name)});
        // TODO handle invalid non-initialized statements
        // search for default value/ddefault method
        break;
    }
    case AST_STMT_VAR_INIT_DECL: {
        OptId<TypeId> maybe_type = TypeResolver<TopLevelDefVisitor>{context, *this}.resolve_type(
            span.file_id, scope, stmt->stmt.var_init_decl.type);
        if (!maybe_type.has_value()) {
            return did; // maybe set a special value to indicate error differently
        }
        if (def.compt && TypeTransformer<TypeContainsVar>{context}(maybe_type.as_id())) {
            context.emplace_diagnostic(context.type(maybe_type.as_id()).span,
                                       diag_code::compt_variable_should_have_an_explicit_type,
                                       diag_type::error);
            return did;
        }
        // compt =/= mut guard
        check_to_err_when_compt_is_not_mut(maybe_type.as_id(), def);

        auto maybe_compt_exec
            = ComptExprSolver(context, *this)
                  .solve_compt_expr(span.file_id, scope, stmt->stmt.var_init_decl.rhs,
                                    maybe_type.as_id());
        if (!maybe_compt_exec.has_value()) {
            if (!def.compt) {
                context.emplace_diagnostic(def.span,
                                           diag_code::even_non_compt_top_levels_need_compt_init,
                                           diag_type::note, DiagnosticInfoNoPreview{});
            }
            return did;
        }
        auto name_sid = context.symbol_id(stmt->stmt.var_init_decl.name);
        def.set_value(DefVariable{
            .type = maybe_type.as_id(), .name = name_sid, .compt_value = maybe_compt_exec.as_id()});
        break;
    }
    case AST_STMT_STRUCT_DEF: {
        auto strct = stmt->stmt.struct_decl;
        // delay resolution/instantiation until specialization
        if (strct.is_generic) {
            return did;
        }
        IdSlice<DefId> ordered_defs = context.ordered_defs_for(did);
        // visit each orderer member (variable), since the struct is actually dependent on those
        // defs
        for (auto didx = ordered_defs.begin(); didx != ordered_defs.end(); didx++) {
            visit_as_dependent(context.def_id(didx));
        }
        SymbolId name = context.symbol_id(strct.name);
        // TODO handle member functions and contracts
        def.set_value(DefStruct{
            .name = name,
            .scope = context.scope_for_top_level_def(did),
            .ordered_members = ordered_defs,
            /* .contracts = */
        });
        break;
    }
    case AST_STMT_CONTRACT_DEF:
    case AST_STMT_UNION_DEF:
    case AST_STMT_VARIANT_DEF:
    case AST_STMT_VARIANT_FIELD_DECL:
    case AST_STMT_FN_DECL:
    case AST_STMT_FN_PROTOTYPE:
    case AST_STMT_DEFTYPE:
        break;

        // the rest are not possible (already handled)/shouldn't be resolved at top level
    case AST_STMT_FILE:
    case AST_STMT_EXTERN_BLOCK:
    case AST_STMT_MODULE:
    case AST_STMT_VISIBILITY_MODIFIER:
    case AST_STMT_COMPT_MODIFIER:
    case AST_STMT_STATIC_MODIFIER:
    case AST_STMT_IMPORT:
    case AST_STMT_USE:
    case AST_STMT_BLOCK:
    case AST_STMT_EXPR:
    case AST_STMT_EMPTY:
    case AST_STMT_BREAK:
    case AST_STMT_IF:
    case AST_STMT_ELSE:
    case AST_STMT_WHILE:
    case AST_STMT_FOR:
    case AST_STMT_FOR_IN:
    case AST_STMT_RETURN:
    case AST_STMT_CONTINUE:
    case AST_STMT_YIELD:
    case AST_STMT_ALIGNAS_MODIFIER:
    case AST_STMT_INVALID:
        break;
    }
    return did;
}

void TopLevelDefVisitor::report_cycle(DefId culprit) {
    // culprit is the origin, but accomplice has just referred to the culprit
    assert(!def_stack.empty());
    DefId accomplice = def_stack[def_stack.size() - 1];

    DiagnosticId accomplice_diag
        = context.emplace_diagnostic(context.make_top_level_def_name_span(accomplice),
                                     diag_code::circular_definition, diag_type::error);

    DiagnosticId prev_diag = accomplice_diag;

    for (auto it = def_stack.rbegin(); it != def_stack.rend(); it++) {
        DefId d = *it;
        if (d != culprit && it != def_stack.rbegin()) {
            DiagnosticId curr_diag = context.emplace_diagnostic(
                context.make_top_level_def_name_span(d), diag_code::circular_definition_passes_thru,
                diag_type::note);
            // set correlation inside of context
            context.set_next_diagnostic(prev_diag, curr_diag);
            // cycle
            prev_diag = curr_diag;

        } else /* is culprit */ {
            DiagnosticId culprit_diag = context.emplace_diagnostic(
                context.make_top_level_def_name_span(culprit),
                diag_code::circular_definition_origin, diag_type::note);
            // set correlation inside of context
            context.set_next_diagnostic(prev_diag, culprit_diag);
            // found, so return out of here
            return;
        }
    }
    // unreachable since we *should* find the culprit in the def_stack
    assert(false && "failed to find culprit defintion when reporting a circular defintion");
}

} // namespace hir
