//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_COMPT_EXPR_SOLVER_HPP
#define COMPILER_HIR_COMPT_EXPR_SOLVER_HPP

#include "compiler/ast/expr.h"
#include "compiler/hir/context.hpp"
#include "compiler/hir/exec.hpp"
#include "compiler/token.h"
#include "def_visitor.hpp"
#include <optional>
namespace hir {

template <IsDefVisitor V> class ComptExprSolver {
    V& def_visitor;
    Context& context;

  public:
    ComptExprSolver(Context& ctx, V& def_visitor) : context{ctx}, def_visitor{def_visitor} {}
    // solves a top level compt expr (this is primarily for array sizing & builtin types for top
    // level generic instantiation with compt parameterizations)
    [[nodiscard]] OptId<ExecId> solve_compt_expr(FileId fid, NamedOrAnonScopeId scope,
                                                 const ast_expr_t* expr, TypeId into_tid) {
        const Type& into_type = context.type(into_tid);
        // try to solve str& at comptime
        if (into_type.holds<TypeRef>()) {
            auto inner = context.type(into_type.as<TypeRef>().inner);
            return (inner.template holds<TypeBuiltin>()
                    && inner.template as<TypeBuiltin>().type == builtin_type::str)
                       ? solve_compt_expr(fid, scope, expr, builtin_type::str)
                       : std::nullopt;
        }
        if (!into_type.holds<TypeBuiltin>()) {
            context.emplace_diagnostic(
                Span(fid, context.ast(fid).buffer(), expr->first, expr->last),
                diag_code::cannot_resolve_at_compt, diag_type::error);
            return OptId<ExecId>{};
        }
        builtin_type into_builtin_type = into_type.as<TypeBuiltin>().type;
        return solve_compt_expr(fid, scope, expr, into_builtin_type);
    }

    [[nodiscard]] OptId<ExecId> solve_compt_expr(FileId fid, NamedOrAnonScopeId scope,
                                                 const ast_expr_t* expr,
                                                 builtin_type into_builtin) {
        auto emplace_e = [&](ExecValue val) {
            return context.register_exec(
                context, val, Span(fid, context.ast(fid).buffer(), expr->first, expr->last), true);
        };

        auto visit_def
            = [&](DefId did) { return context.def(def_visitor.visit_as_dependent(did)); };
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
                // std::cout << def.span.as_sv(context)
                //          << ", resol: " << Def::resol_state_to_str(context.resol_state_of(did))
                //          << '\n';
                if (def.holds<DefVariable>() && def.compt) {
                    maybe_value = context.exec(def.as<DefVariable>().compt_value.as_id())
                                      .template as<ExecExprComptConstant>();
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
            context.emplace_diagnostic(
                Span(fid, context.ast(fid).buffer(), expr->first, expr->last),
                diag_code::cannot_resolve_at_compt, diag_type::error);
            goto ret_without_diag;
        }
        if (maybe_value.has_value()) {
            // std::cout << builtin_type_to_cstr(maybe_value.value().type()) << '\n'; // debug
            auto maybe_converted = maybe_value.value().try_implicit_convert_to(into_builtin);
            if (!maybe_converted.has_value()) {
                context.emplace_diagnostic(
                    Span(fid, context.ast(fid).buffer(), expr->first, expr->last),
                    diag_code::cannot_convert_to_some_builtin_type, diag_type::error,
                    DiagnosticCannotConvertToBuiltinType{.type = into_builtin});
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
};
} // namespace hir
#endif
