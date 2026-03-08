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
#include "compiler/hir/def.hpp"
#include "compiler/hir/diagnostic.hpp"
#include "compiler/hir/exec.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/hir/type.hpp"
#include "compiler/token.h"
#include <cassert>
#include <iostream>
#include <optional>

namespace hir {

void TopLevelDefVisitor::resolve_top_level_definitions() {
    assert(!this->began_resolution && "already began resolving top level declarations");
    this->began_resolution = true;
    auto last_top_level = context.defs.end_id();
    for (auto d = context.defs.begin_id(); d != last_top_level; d++) {
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
    const ast_stmt* stmt = context.def_ast_nodes.cat(did);
    auto scope = context.containing_scope(did);
    Def& def = context.def(did);
    Span span = def.span;
    //  TODO write handlers
    switch (stmt->type) {
    case AST_STMT_FILE:
    case AST_STMT_EXTERN_BLOCK:
    case AST_STMT_VAR_DECL: {
        OptId<TypeId> maybe_type = TopLevelTypeResolver{context, *this}.resolve_type(
            span.file_id, scope, stmt->stmt.var_decl.type);
        if (!maybe_type.has_value()) {
            return did; // maybe set a special value to indicate error differently
        }
        def.set_value(DefVariable{.type = maybe_type.as_id(),
                                  .name = context.symbol_id(stmt->stmt.var_decl.name)});
        // TODO handle invalid non-initialized statements
        break;
    }
    case AST_STMT_VAR_INIT_DECL: {
        OptId<TypeId> maybe_type = TopLevelTypeResolver{context, *this}.resolve_type(
            span.file_id, scope, stmt->stmt.var_init_decl.type);
        if (!maybe_type.has_value()) {
            return did; // maybe set a special value to indicate error differently
        }
        auto maybe_compt_exec
            = TopLevelConstantExprSolver(context, *this)
                  .solve_compt_expr(span.file_id, scope, stmt->stmt.var_init_decl.rhs,
                                    maybe_type.as_id());
        if (!maybe_compt_exec.has_value()) {
            return did;
        }
        def.set_value(DefVariable{.type = maybe_type.as_id(),
                                  .name = context.symbol_id(stmt->stmt.var_init_decl.name),
                                  .compt_value = maybe_compt_exec.as_id()});
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
    case AST_STMT_YIELD:
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
                                                           builtin_type into_builtin) {
    auto emplace_e = [&](ExecValue val) {
        return context.execs.emplace_and_get_id(
            context, val, Span(fid, context.ast(fid).buffer(), expr->first, expr->last), true);
    };

    auto visit_def = [&](DefId did) { return context.def(def_visitor.visit_as_dependent(did)); };
    // TODO, finish handling based on builtin_type and the strucuture of the expr (recurse)
    std::optional<ExecExprComptConstant> maybe_value;
    switch (expr->type) {
    case AST_EXPR_ID: {
        auto maybe_def
            = context.look_up_scoped_variable(scope, context.symbol_slice(expr->expr.id.slice));
        if (maybe_def.has_value()) {
            // happy path, canonicalize compt value
            DefId did = maybe_def.as_id();
            const Def& def = visit_def(did);
            std::cout << def.span.as_sv(context)
                      << ", resol: " << Def::resol_state_to_str(context.resol_state_of(did))
                      << '\n';
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
            maybe_value = ExecExprComptConstant{tkn->val.signed_integral};
            break;
        case TOK_UINT_LIT:
            maybe_value = ExecExprComptConstant{tkn->val.unsigned_integral};
            break;
        case TOK_FLOAT_LIT:
            maybe_value = ExecExprComptConstant{static_cast<float>(tkn->val.floating)};
            break;
        case TOK_STR_LIT:
            maybe_value = ExecExprComptConstant{context.symbol_id_for_str_lit_tkn(tkn)};
            break;
        case TOK_BOOL_LIT_FALSE:
            maybe_value = ExecExprComptConstant{false};
            break;
        case TOK_BOOL_LIT_TRUE:
            maybe_value = ExecExprComptConstant{true};
            break;
        case TOK_NULL_LIT:
            maybe_value = ExecExprComptConstant{nullptr};
            break;
        default:
            std::unreachable();
            break;
        }
        break;
    }
    case AST_EXPR_BINARY: {
        // TODO operator and type handlers
        break;
    }
    case AST_EXPR_GROUPING: {
        // get inner
        return solve_compt_expr(fid, scope, expr->expr.grouping.expr, into_builtin);
        break;
    }
    case AST_EXPR_PRE_UNARY: {
        // eventually allow sizeof, allignof
        // TODO handle bool not
        if (expr->expr.unary.op->type == TOK_BOOL_NOT) {
            auto inner = solve_compt_expr(fid, scope, expr->expr.unary.expr, into_builtin);
        }
    }
    case AST_EXPR_POST_UNARY:
        // not supported (-- or ++ require mutable lvalues)
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
        // std::cout << builtin_type_to_cstr(maybe_value.value().type()) << '\n'; // debug
        auto maybe_converted = maybe_value.value().try_implicit_convert_to(into_builtin);
        if (!maybe_converted.has_value()) {
            context.emplace_diagnostic(
                Span(fid, context.ast(fid).buffer(), expr->first, expr->last),
                diag_code::cannot_convert_to_some_builtin_type, diag_type::error);
            return OptId<ExecId>{};
        }
        // std::cout << builtin_type_to_cstr(maybe_converted.value().type()) << '\n'; // debug
        assert(maybe_converted.value().matches_type(into_builtin));
        return emplace_e(maybe_converted.value());
    }
    context.emplace_diagnostic(Span(fid, context.ast(fid).buffer(), expr->first, expr->last),
                               diag_code::cannot_resolve_at_compt, diag_type::error);
// as to not duplicate compt errors from bubbling up
ret_without_diag:
    return OptId<ExecId>{};
}
OptId<ExecId> TopLevelConstantExprSolver::solve_compt_expr(FileId fid, NamedOrAnonScopeId scope,
                                                           const ast_expr_t* expr,
                                                           TypeId into_tid) {
    const Type& into_type = context.type(into_tid);
    // try to solve str& at comptime
    if (into_type.holds<TypeRef>()) {
        auto inner = context.type(into_type.as<TypeRef>().inner);
        return (inner.holds<TypeBuiltin>() && inner.as<TypeBuiltin>().type == builtin_type::str)
                   ? solve_compt_expr(fid, scope, expr, builtin_type::str)
                   : std::nullopt;
    }
    if (!into_type.holds<TypeBuiltin>()) {
        context.emplace_diagnostic(Span(fid, context.ast(fid).buffer(), expr->first, expr->last),
                                   diag_code::cannot_resolve_at_compt, diag_type::error);
        return OptId<ExecId>{};
    }
    builtin_type into_builtin_type = into_type.as<TypeBuiltin>().type;
    return solve_compt_expr(fid, scope, expr, into_builtin_type);
}

[[nodiscard]] OptId<TypeId> TopLevelTypeResolver::resolve_type(FileId fid, NamedOrAnonScopeId scope,
                                                               const ast_type_t* type) {
    return resolve_type(fid, scope, type, false);
}

[[nodiscard]] OptId<TypeId> TopLevelTypeResolver::resolve_type(FileId fid, NamedOrAnonScopeId scope,
                                                               const ast_type_t* type,
                                                               bool visit_as_transparent) {
    switch (type->tag) {
    case AST_TYPE_BASE: {
        return type_base(fid, scope, type, visit_as_transparent);
    }
    case AST_TYPE_REF_PTR: {
        return type_ptr_ref(fid, scope, type, visit_as_transparent);
    }
    case AST_TYPE_ARR:
        return type_arr(fid, scope, type, visit_as_transparent);
    case AST_TYPE_SLICE:
        return type_slice(fid, scope, type, visit_as_transparent);
    case AST_TYPE_GENERIC: {
        // TODO, attempt a generic instantiation
        break;
    }
    case AST_TYPE_FN_PTR:
        return type_fn_ptr(fid, scope, type, visit_as_transparent);
    case AST_TYPE_VARIADIC:
        return type_variadic(fid, scope, type, visit_as_transparent);
    case AST_TYPE_INVALID:
        break;
    }
    return OptId<TypeId>{};
}

[[nodiscard]] OptId<TypeId> TopLevelTypeResolver::type_base(FileId fid, NamedOrAnonScopeId scope,
                                                            const ast_type_t* type,
                                                            bool visit_as_transparent) {
    auto maybe_builtin = id_tkn_slice_to_maybe_builtin(type->type.base.id);
    if (maybe_builtin.has_value()) {
        return context.emplace_type(TypeBuiltin{maybe_builtin.value()},
                                    Span(context, fid, type->first, type->last),
                                    type->type.base.mut);
    }
    OptId<DefId> maybe_structure
        = context.look_up_scoped_type(scope, context.symbol_slice(type->type.base.id));
    if (maybe_structure.has_value()) {
        auto def = visit_as_transparent ? def_visitor.visit_as_transparent(maybe_structure.as_id())
                                        : def_visitor.visit_as_dependent(maybe_structure.as_id());
        return context.emplace_type(TypeStructure{def}, Span(context, fid, type->first, type->last),
                                    type->type.base.mut);
    }
    // not builtin and no struct, variant, or union found
    context.emplace_diagnostic(Span(context, fid, type->first, type->last),
                               diag_code::type_not_defined, diag_type::error);
    return OptId<TypeId>{};
}

OptId<TypeId> TopLevelTypeResolver::type_ptr_ref(FileId fid, NamedOrAnonScopeId scope,
                                                 const ast_type_t* type,
                                                 bool visit_as_transparent) {
    auto maybe_inner = resolve_type(fid, scope, type->type.ptr_ref.inner,
                                    true); // inner always transparent thru a ref/ptr
    if (!maybe_inner.has_value()) {
        return OptId<TypeId>{};
    }
    // inner*
    if (type->type.ptr_ref.modifier->type == TOK_STAR) {
        return context.emplace_type(TypePtr{.inner = maybe_inner.as_id()},
                                    Span(context, fid, type->first, type->last),
                                    type->type.ptr_ref.mut);
    }
    // inner&
    return context.emplace_type(TypeRef{.inner = maybe_inner.as_id()},
                                Span(context, fid, type->first, type->last),
                                type->type.ptr_ref.mut);
}

OptId<TypeId> TopLevelTypeResolver::type_arr(FileId fid, NamedOrAnonScopeId scope,
                                             const ast_type_t* type, bool visit_as_transparent) {
    auto maybe_inner = resolve_type(fid, scope, type->type.arr.inner, visit_as_transparent);
    if (!maybe_inner.has_value()) {
        return OptId<TypeId>{};
    }
    auto maybe_size_exec = TopLevelConstantExprSolver{context, def_visitor}.solve_compt_expr(
        fid, scope, type->type.arr.size_expr, builtin_type::usize);
    if (!maybe_size_exec.has_value()) {
        return OptId<TypeId>{};
    }
    auto size = context.exec(maybe_size_exec.as_id()).as<ExecExprComptConstant>().as<size_t>();
    return context.emplace_type(TypeArr{.inner = maybe_inner.as_id(),
                                        .compt_size_expr = maybe_size_exec.as_id(),
                                        .canonical_size = size},
                                Span(context, fid, type->first, type->last), false);
}

OptId<TypeId> TopLevelTypeResolver::type_slice(FileId fid, NamedOrAnonScopeId scope,
                                               const ast_type_t* type, bool visit_as_transparent) {
    auto maybe_inner
        = resolve_type(fid, scope, type->type.slice.inner,
                       true); // thru slice is always transparent (since it's a wide ptr)
    if (!maybe_inner.has_value()) {
        return OptId<TypeId>{};
    }
    return context.emplace_type(TypeSlice{.inner = maybe_inner.as_id()},
                                Span(context, fid, type->first, type->last), type->type.slice.mut);
}

OptId<TypeId> TopLevelTypeResolver::type_fn_ptr(FileId fid, NamedOrAnonScopeId scope,
                                                const ast_type_t* type, bool visit_as_transparent) {
    auto maybe_return_type = resolve_type(fid, scope, type->type.fn_ptr.return_type,
                                          true); // thru fn ptr is always transparent
    if (!maybe_return_type.has_value()) {
        return OptId<TypeId>{};
    }
    auto return_type = maybe_return_type.as_id();
    llvm::SmallVector<TypeId> tid_vec;
    auto ast_param_type_slice = type->type.fn_ptr.param_types;
    // get slice of of params
    for (size_t i = 0; i < ast_param_type_slice.len; i++) {
        auto maybe_tid = resolve_type(fid, scope, ast_param_type_slice.start[i]);
        if (!maybe_tid.has_value()) {
            return OptId<TypeId>{};
        }
        tid_vec.push_back(maybe_tid.as_id());
    }
    IdSlice<TypeId> param_tid_slice = context.freeze_id_vec(tid_vec);
    return context.emplace_type(
        TypeFnPtr{.param_types = param_tid_slice, .return_type = return_type},
        Span(context, fid, type->first, type->last), type->type.slice.mut);
}
OptId<TypeId> TopLevelTypeResolver::type_variadic(FileId fid, NamedOrAnonScopeId scope,
                                                  const ast_type_t* type,
                                                  bool visit_as_transparent) {
    auto maybe_inner = resolve_type(
        fid, scope, type->type.variadic.inner,
        true); // thru variadic is always transparent (since it's effectively a slice)
    if (!maybe_inner.has_value()) {
        return OptId<TypeId>{};
    }
    return context.emplace_type(TypeVariadic{.inner = maybe_inner.as_id()},
                                Span(context, fid, type->first, type->last),
                                false); // mut bound to variadic is not possible
}
} // namespace hir
