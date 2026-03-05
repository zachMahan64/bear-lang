//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/top_level_def_visitor.hpp"
#include "compiler/ast/expr.h"
#include "compiler/hir/context.hpp"
#include "compiler/hir/diagnostic.hpp"
#include "compiler/hir/exec.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/hir/type.hpp"
#include <cassert>

namespace hir {

void TopLevelDefVisitor::resolve_top_level_definitions() {
    assert(!this->began_resolution && "already began resolving top level declarations");
    this->began_resolution = true;
    for (auto d = context.defs.begin_id(); d != context.defs.end_id(); d++) {
        resolve_def(d);
    }
}
// TODO allow for passing of ast_generic_args_t* for generic instatiations?
DefId TopLevelDefVisitor::visit(DefId def) {
    if (context.resol_state_of(def) == Def::resol_state::resolved) {
        return def;
    }
    def_stack.push_back(def);
    auto d = resolve_def(def);
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

DefId TopLevelDefVisitor::resolve_def(DefId def) {
    const ast_stmt* stmt = context.def_ast_nodes.cat(def);
    ScopeId scope = context.scope_for_top_level_def(def);
    // TODO write handlers
    switch (stmt->type) {
    case AST_STMT_FILE:
    case AST_STMT_EXTERN_BLOCK:
    case AST_STMT_VAR_DECL:
    case AST_STMT_VAR_INIT_DECL:
    case AST_STMT_MODULE:
    case AST_STMT_VISIBILITY_MODIFIER:
    case AST_STMT_COMPT_MODIFIER:
    case AST_STMT_STATIC_MODIFIER:
    case AST_STMT_STRUCT_DEF:
    case AST_STMT_CONTRACT_DEF:
    case AST_STMT_UNION_DEF:
    case AST_STMT_VARIANT_DEF:
    case AST_STMT_VARIANT_FIELD_DECL:
    case AST_STMT_FN_DECL:
    case AST_STMT_FN_PROTOTYPE:
    case AST_STMT_DEFTYPE:
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
    case AST_STMT_YIELD:
    case AST_STMT_INVALID:
        break;
    }
    return def;
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
        if (d != culprit) {
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

OptId<ExecId> TopLevelConstantExprSolver::solve_compt_expr(FileId fid, NamedOrAnonScopeId scope,
                                                           const ast_expr_t* expr,
                                                           TypeId into_tid) {
    auto emplace_e = [&](ExecValue val) {
        return context.execs.emplace_and_get_id(
            context, val, Span(fid, context.ast(fid).buffer(), expr->first, expr->last), true);
    };

    auto visit_def = [&](DefId did) { return context.def(def_visitor.visit_as_dependent(did)); };

    const Type& into_type = context.type(into_tid);
    if (!into_type.holds<TypeBuiltin>()) {
        context.emplace_diagnostic(Span(fid, context.ast(fid).buffer(), expr->first, expr->last),
                                   diag_code::cannot_resolve_at_compt, diag_type::error);
        return OptId<ExecId>{};
    }
    builtin_type into_builtin_type = into_type.as<TypeBuiltin>().type;
    // TODO, finish handling based on builtin_type and the strucuture of the expr (recurse)
    std::optional<ExecExprComptConstant> maybe_value;
    switch (expr->type) {
    case AST_EXPR_ID: {
        auto maybe_def = context.find_variable_from_scoped_id(
            scope, context.symbol_slice(expr->expr.id.slice));
        if (maybe_def.has_value()) {
            // happy path, canonicalize compt value
            DefId did = maybe_def.as_id();
            const Def& def = visit_def(did);
            if (def.holds<DefVariable>() && def.compt) {
                maybe_value = context.execs.cat(def.as<DefVariable>().compt_value.as_id())
                                  .as<ExecExprComptConstant>();
            }
        }
        break;
    }
    case AST_EXPR_LITERAL: {
        const token_t* tkn = expr->expr.literal.tkn;
        // TODO, handle
        switch (tkn->type) {
        case TOK_CHAR_LIT:
            maybe_value = ExecExprComptConstant{tkn->val.character};
            break;
        case TOK_INT_LIT:
            maybe_value = ExecExprComptConstant{tkn->val.integral};
            break;
        case TOK_FLOAT_LIT:
            maybe_value = ExecExprComptConstant{static_cast<float>(tkn->val.floating)};
            break;
        case TOK_STR_LIT:
            maybe_value = ExecExprComptConstant{context.symbol_id(tkn)};
            break;
        case TOK_BOOL_LIT_FALSE:
            maybe_value = ExecExprComptConstant{false};
            break;
        case TOK_BOOL_LIT_TRUE:
            maybe_value = ExecExprComptConstant{true};
        case TOK_NULL_LIT:
            maybe_value = ExecExprComptConstant{nullptr};
            break;
        default:
            std::unreachable();
            break;
        }
    }
    // TODO -----------------
    case AST_EXPR_BINARY: {
        // TODO operator and type handlers
        break;
    }
    case AST_EXPR_GROUPING: {
        // get inner
        return solve_compt_expr(fid, scope, expr->expr.grouping.expr, into_tid);
        break;
    }
    case AST_EXPR_PRE_UNARY:
    case AST_EXPR_POST_UNARY:
    // TODO ^^^^^^^^^^^^^^^^^
    case AST_EXPR_SUBSCRIPT:
    case AST_EXPR_FN_CALL:
    case AST_EXPR_TYPE:
    case AST_EXPR_BORROW:
    case AST_EXPR_LIST_LITERAL:
    case AST_EXPR_STRUCT_INIT:
    case AST_EXPR_STRUCT_MEMBER_INIT:
    case AST_EXPR_CLOSURE:
    case AST_EXPR_VARIANT_DECOMP:
    case AST_EXPR_BLOCK:
    case AST_EXPR_MATCH_BRANCH:
    case AST_EXPR_MATCH:
    case AST_EXPR_ELSE_MATCH_BRANCH:
    case AST_EXPR_INVALID:
        // not a valid compile-time expr
        context.emplace_diagnostic(Span(fid, context.ast(fid).buffer(), expr->first, expr->last),
                                   diag_code::cannot_resolve_at_compt, diag_type::error);
        goto ret_without_diag;
    }
    if (maybe_value.has_value()) {
        auto maybe_converted = maybe_value.value().try_up_convert_to(into_builtin_type);
        if (!maybe_value->matches_type(into_builtin_type)) {
            context.emplace_diagnostic(
                Span(fid, context.ast(fid).buffer(), expr->first, expr->last),
                diag_code::cannot_convert_to_some_builtin_type, diag_type::error);
            return OptId<ExecId>{};
        }
        assert(maybe_converted.value().matches_type(into_builtin_type));
        return emplace_e(maybe_converted.value());
    }
    context.emplace_diagnostic(Span(fid, context.ast(fid).buffer(), expr->first, expr->last),
                               diag_code::cannot_resolve_at_compt, diag_type::error);
// as to not duplicate compt errors from bubbling up
ret_without_diag:
    return OptId<ExecId>{};
}

[[nodiscard]] OptId<ExecId> TopLevelTypeResolver::resolve_type(FileId fid, NamedOrAnonScopeId scope,
                                                               const ast_type_t* type) {
    // TODO
    switch (type->tag) {
    case AST_TYPE_BASE:
    case AST_TYPE_REF_PTR:
    case AST_TYPE_ARR:
    case AST_TYPE_SLICE:
    case AST_TYPE_GENERIC:
    case AST_TYPE_FN_PTR:
    case AST_TYPE_VARIADIC:
    case AST_TYPE_INVALID:
        break;
    }
}

} // namespace hir
