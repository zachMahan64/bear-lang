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
#include "compiler/hir/def.hpp"
#include "compiler/hir/diagnostic.hpp"
#include "compiler/hir/exec.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/hir/type.hpp"
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
                       ? solve_builtin_compt_expr(fid, scope, expr, builtin_type::str)
                       : std::nullopt;
        }
        if (into_type.holds<TypeStructure>()) {
            return solve_struct_compt_expr(fid, scope, expr, into_tid);
        }
        // guard against non-builtins
        if (!into_type.holds<TypeBuiltin>()) {
            context.emplace_diagnostic(
                Span(fid, context.ast(fid).buffer(), expr->first, expr->last),
                diag_code::cannot_resolve_at_compt, diag_type::error);
            return OptId<ExecId>{};
        }
        builtin_type into_builtin_type = into_type.as<TypeBuiltin>().type;
        return solve_builtin_compt_expr(fid, scope, expr, into_builtin_type);
    }

    [[nodiscard]] OptId<ExecId> solve_builtin_compt_expr(FileId fid, NamedOrAnonScopeId scope,
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
                if (def.holds<DefVariable>()) {
                    if (!def.compt) {
                        auto diag_id = context.emplace_diagnostic(
                            Span(fid, context.ast(fid).buffer(), expr->first, expr->last),
                            diag_code::cannot_init_with_non_compt_value, diag_type::error,
                            DiagnosticSubCode{.sub_code = diag_code::not_a_compile_time_constant});
                        auto sub_diag_id = context.emplace_diagnostic(
                            def.span, diag_code::declared_here_without_compt, diag_type::note);
                        context.set_next_diagnostic(diag_id, sub_diag_id);
                        return std::nullopt;
                    }
                    maybe_value = context.exec(def.as<DefVariable>().compt_value.as_id())
                                      .template as<ExecExprComptConstant>();
                }
            } else {
                auto sid_slice = context.symbol_slice(expr->expr.id.slice);
                auto diag_id = context.emplace_diagnostic(
                    Span(fid, context.ast(fid).buffer(), expr->first, expr->last),
                    diag_code::use_of_undeclared_identifier, diag_type::error,
                    DiagnosticIdentifierAfterMessage{.sid_slice = sid_slice},
                    DiagnosticSubCode{.sub_code = diag_code::not_declared_in_this_scope});
                return std::nullopt;
            }
            break;
        }
        case AST_EXPR_LITERAL: {
            const token_t* tkn = expr->expr.literal.tkn;
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
                maybe_value = ExecExprComptConstant{tkn->val.floating};
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
            OptId<ExecId> lhs
                = solve_builtin_compt_expr(fid, scope, expr->expr.binary.lhs, into_builtin);
            if (!lhs.has_value()) {
                return std::nullopt;
            }
            OptId<ExecId> rhs
                = solve_builtin_compt_expr(fid, scope, expr->expr.binary.rhs, into_builtin);
            if (!rhs.has_value()) {
                return std::nullopt;
            }
            break;
        }
        case AST_EXPR_GROUPING: {
            // get inner
            return solve_builtin_compt_expr(fid, scope, expr->expr.grouping.expr, into_builtin);
            break;
        }
        case AST_EXPR_PRE_UNARY: {
            // eventually allow sizeof, allignof
            if (expr->expr.unary.op->type == TOK_BOOL_NOT) {
                auto inner
                    = solve_builtin_compt_expr(fid, scope, expr->expr.unary.expr, into_builtin);
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
            if (maybe_value->holds<SymbolId>()) {
            }
            if (!maybe_converted.has_value()) {
                auto from_builtin = maybe_value.value().type_builtin();
                TypeId from = context.emplace_type(TypeBuiltin{.type = from_builtin},
                                                   Span::generated(), false);
                TypeId to = context.emplace_type(TypeBuiltin{.type = into_builtin},
                                                 Span::generated(), false);
                context.emplace_diagnostic(
                    Span(fid, context.ast(fid).buffer(), expr->first, expr->last),
                    diag_code::cannot_convert_type, diag_type::error,
                    DiagnosticCannotConvertFromTypeToType{.from = from, .to = to});
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
    [[nodiscard]] OptId<ExecId> solve_struct_compt_expr(FileId fid, NamedOrAnonScopeId scope,
                                                        const ast_expr_t* expr, TypeId into_tid) {
        auto emplace_e = [&](ExecValue val) {
            return context.register_exec(
                context, val, Span(fid, context.ast(fid).buffer(), expr->first, expr->last), true);
        };

        auto visit_def
            = [&](DefId did) { return context.def(def_visitor.visit_as_dependent(did)); };

        auto expr_span = Span(fid, context.ast(fid).buffer(), expr->first, expr->last);

        switch (expr->type) {
        case AST_EXPR_ID: {
            auto maybe_def
                = context.look_up_scoped_variable(scope, context.symbol_slice(expr->expr.id.slice));
            if (!maybe_def.has_value()) {
                auto sid_slice = context.symbol_slice(expr->expr.id.slice);
                context.emplace_diagnostic(
                    expr_span, diag_code::use_of_undeclared_identifier, diag_type::error,
                    DiagnosticIdentifierAfterMessage{.sid_slice = sid_slice},
                    DiagnosticSubCode{.sub_code = diag_code::not_declared_in_this_scope});
                return std::nullopt;
            }
            // happy path, canonicalize compt value
            DefId did = maybe_def.as_id();
            const Def& def = visit_def(did);
            if (!def.holds<DefVariable>()) {
                auto sid_slice = context.symbol_slice(expr->expr.id.slice);
                context.emplace_diagnostic(
                    expr_span, diag_code::cannot_convert_expression_to_type, diag_type::error,
                    DiagnosticCannotConvertToType{.tid = into_tid}, DiagnosticNoOtherInfo{});
                return std::nullopt;
            }
            const auto& def_variable = def.as<DefVariable>();
            if (!def.compt) {
                auto diag_id = context.emplace_diagnostic(
                    expr_span, diag_code::cannot_init_with_non_compt_value, diag_type::error,
                    DiagnosticSubCode{.sub_code = diag_code::not_a_compile_time_constant});
                auto sub_diag_id = context.emplace_diagnostic(
                    def.span, diag_code::declared_here_without_compt, diag_type::note);
                context.set_next_diagnostic(diag_id, sub_diag_id);
                return std::nullopt;
            }
            if (!def_variable.compt_value.has_value()) {
                // this means that this definition must have had an issue, and it was thus already
                // reported, so just return a nullopt
                return std::nullopt;
            }
            if (context.equivalent_type(def_variable.type, into_tid)) {
                // since it's compt (immutable, it's perfectly fine to just share the
                // original exec value here)
                return def_variable.compt_value;
            }
            context.emplace_diagnostic(
                expr_span, diag_code::cannot_convert_type, diag_type::error,
                DiagnosticCannotConvertFromTypeToType{.from = def_variable.type, .to = into_tid});
            return std::nullopt;
        }
        case AST_EXPR_STRUCT_INIT: {

            auto id_slice = expr->expr.struct_init.id;
            auto sid_slice = context.symbol_slice(expr->expr.struct_init.id);

            OptId<DefId> maybe_def_of_struct = context.look_up_scoped_type(scope, sid_slice);

            if (!maybe_def_of_struct.has_value()) {
                context.emplace_diagnostic(
                    expr_span, diag_code::use_of_undeclared_identifier, diag_type::error,
                    DiagnosticIdentifierAfterMessage{.sid_slice = sid_slice},
                    DiagnosticSubCode{.sub_code = diag_code::not_declared_in_this_scope});
                return std::nullopt;
            }

            const auto did = maybe_def_of_struct.as_id();
            const Def& def = context.def(did);

            if (context.type(into_tid).template holds<TypeGenericStructure>()) {
                auto did0 = context.emplace_diagnostic(
                    expr_span, diag_code::compt_generic_structs_not_possible, diag_type::error);
                auto did1
                    = context.emplace_diagnostic(expr_span, diag_code::this_feature_is_planned,
                                                 diag_type::note, DiagnosticInfoNoPreview{});
                auto did2
                    = context.emplace_diagnostic(expr_span, diag_code::remove, diag_type::help,
                                                 DiagnosticSymbolAfterMessageNoQuotes{
                                                     context.symbol_id("the struct initializer")},
                                                 DiagnosticNoOtherInfo{});
                context.set_next_diagnostic(did0, did1);
                context.set_next_diagnostic(did1, did2);
                return std::nullopt;
            }
            if (!def.holds<DefStruct>()) {
                auto did0 = context.emplace_diagnostic(
                    Span(fid, context.ast(fid).buffer(), id_slice.start[0],
                         id_slice.start[id_slice.len - 1]),
                    diag_code::is_not_a_struct, diag_type::error,
                    DiagnosticIdentifierBeforeMessage{.sid_slice = sid_slice},
                    DiagnosticNoOtherInfo{});

                auto did1 = context.emplace_diagnostic(def.span, diag_code::declared_here,
                                                       diag_type::note);
                context.set_next_diagnostic(did0, did1);
                return std::nullopt;
            }

            // somewhat validated now
            const DefStruct& def_as_struct = def.as<DefStruct>();
            const auto defs = context.ordered_defs_for(did);

            const ast_slice_of_exprs_t init_slice = expr->expr.struct_init.member_inits;

            enum class relative_arity : uint8_t { too_few, same, too_many };

            relative_arity rel_arity = relative_arity::same;
            if (init_slice.len < defs.len()) {
                rel_arity = relative_arity::too_few;
            } else if (init_slice.len > defs.len()) {
                rel_arity = relative_arity::too_many;
            }

            llvm::SmallVector<ExecId> member_init_execs;
            bool cooked = false;
            for (auto i = 0u; i < defs.len(); i++) {
                auto didx = defs.get(i);
                const auto member_did = context.def_id(didx);
                const Def& member = context.def(didx);

                assert(member.holds<DefVariable>());
                const auto& member_as_var = member.as<DefVariable>();

                const TypeId member_type = member_as_var.type;
                const OptId<ExecId> default_val = member_as_var.compt_value;
                const SymbolId member_name = member.name;
                const ScopeId struct_scope = context.scope_for_top_level_def(did);

                // handle too few
                if (i >= init_slice.len) {
                    // get default value for the member field
                    if (default_val.has_value()) {
                        ExecExprStructMemberInit init{
                            .field_def = did, .value = default_val.as_id(), .move = false};
                        member_init_execs.emplace_back(
                            context.register_exec(context, init, Span::generated(), true));
                    } else {
                        cooked = true;
                        auto did0 = context.emplace_diagnostic(
                            Span(fid, context.ast(fid).buffer(), expr->last),
                            diag_code::struct_field_not_initialized, diag_type::error,
                            DiagnosticSymbolAfterMessage{.sid = context.def(defs.get(i)).name},
                            DiagnosticNoOtherInfo{});
                    }
                    continue; // guard overflow
                }
                assert(i < init_slice.len);
                const ast_expr_t* member_init_expr = init_slice.start[i];

                if (member_init_expr->type != AST_EXPR_STRUCT_MEMBER_INIT) {
                    return std::nullopt; // malformed, so was already reported by parser
                }
                const token_t* proposed_member_name_tkn
                    = member_init_expr->expr.struct_member_init.id;
                const token_t* assign_op = member_init_expr->expr.struct_member_init.id;

                if (assign_op->type == TOK_ASSIGN_MOVE) {
                    context.emplace_diagnostic(Span(fid, context.ast(fid).buffer(), assign_op),
                                               diag_code::compt_values_cannot_be_moved,
                                               diag_type::error);
                }
                const ast_expr_t* proposed_val = member_init_expr->expr.struct_member_init.value;
                const Span proposed_member_span
                    = Span(fid, context.ast(fid).buffer(), member_init_expr->first,
                           member_init_expr->last);

                const SymbolId true_name = member_as_var.name;
                if (context.symbol_id(proposed_member_name_tkn) != true_name) {
                    cooked = true;
                    context.emplace_diagnostic(
                        proposed_member_span, diag_code::field_initializer_does_not_match_field,
                        diag_type::error, DiagnosticSymbolAfterMessage{member_as_var.name},
                        DiagnosticNoOtherInfo{});
                    continue;
                }
                OptId<ExecId> hopefully_exec
                    = solve_compt_expr(fid, scope, proposed_val, member_type);
                if (!hopefully_exec.has_value()) {
                    cooked = true;
                    // just continue, caused by other so must have already been reported
                    continue;
                }
                // emplace the init execs
                member_init_execs.emplace_back(context.emplace_exec(
                    ExecExprStructMemberInit{
                        .field_def = member_did, .value = hopefully_exec.as_id(), .move = false},
                    proposed_member_span, true));
            }
            if (rel_arity == relative_arity::too_many) {
                cooked = true;
                const token_t* first = init_slice.start[defs.len()]->first;
                const token_t* last = init_slice.start[init_slice.len - 1]->last;
                context.emplace_diagnostic(Span(fid, context.ast(fid).buffer(), first, last),
                                           diag_code::too_many_initializers_given_for_struct_init,
                                           diag_type::error);
            }
            if (cooked) {
                context.emplace_diagnostic(
                    def.span, diag_code::declared_here, diag_type::note,
                    DiagnosticIdentifierBeforeMessage{.sid_slice = sid_slice},
                    DiagnosticNoOtherInfo{});
                return std::nullopt;
            }
            // type check before returning here!
            auto into_did = context.type(into_tid).template as<TypeStructure>().definition;
            if (did != into_did) {
                context.emplace_diagnostic(
                    expr_span, diag_code::cannot_convert_type, diag_type::error,
                    DiagnosticCannotConvertFromTypeToType{
                        .from = context.emplace_type(
                            TypeStructure{.definition = did},
                            Span(
                                fid, context.ast(fid).buffer(), expr->expr.struct_init.id.start[0],
                                expr->expr.struct_init.id.start[expr->expr.struct_init.id.len - 1]),
                            false),
                        .to = into_tid});
                return std::nullopt;
            }

            // all good, return the exec
            return context.emplace_exec(
                ExecExprStructInit{.member_inits = context.freeze_id_vec(member_init_execs),
                                   .struct_def = did},
                expr_span, true);
        }

        // these are all invalid
        case AST_EXPR_LITERAL:
        case AST_EXPR_LIST_LITERAL:
        case AST_EXPR_BINARY:
        case AST_EXPR_GROUPING:
        case AST_EXPR_PRE_UNARY:
        case AST_EXPR_POST_UNARY:
        case AST_EXPR_SUBSCRIPT:
        case AST_EXPR_FN_CALL:
        case AST_EXPR_TYPE:
        case AST_EXPR_BORROW:
        case AST_EXPR_STRUCT_MEMBER_INIT:
        case AST_EXPR_CLOSURE:
        case AST_EXPR_VARIANT_DECOMP:
        case AST_EXPR_BLOCK:
        case AST_EXPR_MATCH_BRANCH:
        case AST_EXPR_MATCH:
        case AST_EXPR_ELSE_MATCH_BRANCH:
        case AST_EXPR_INVALID:
            break;
        }
        context.emplace_diagnostic(expr_span, diag_code::cannot_convert_expression_to_type,
                                   diag_type::error, DiagnosticCannotConvertToType{into_tid},
                                   DiagnosticNoOtherInfo{});
    ret:
        return std::nullopt;
    }
};
} // namespace hir
#endif
