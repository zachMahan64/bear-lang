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
#include "compiler/hir/exec_ops.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/hir/type.hpp"
#include "compiler/token.h"
#include "def_visitor.hpp"
#include <iostream>
#include <optional>
#include <utility>
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
            auto inner_tid = into_type.as<TypeRef>().inner;
            auto inner = context.type(inner_tid);
            if (inner.template holds<TypeBuiltin>()
                && inner.template as<TypeBuiltin>().type == builtin_type::str) {
                return solve_builtin_compt_expr(fid, scope, expr, builtin_type::str);
            }
            auto did0 = context.emplace_diagnostic(
                into_type.span, diag_code::type_is_not_resolvable_at_compt, diag_type::error);
            if (inner.template holds_any_of<TypeStructure, TypeBuiltin>()) {
                auto did1 = context.emplace_diagnostic_with_message_value(
                    into_type.span, diag_code::replace_with, diag_type::help,
                    DiagnosticTypeAfterMessage{.tid = inner_tid});
                context.set_next_diagnostic(did0, did1);
            }
            return std::nullopt;
        }
        if (into_type.holds<TypeStructure>()) {
            return solve_struct_compt_expr(fid, scope, expr, into_tid);
        }

        if (into_type.holds<TypeVar>()) {
            if (expr->type == AST_EXPR_STRUCT_INIT) {
                return solve_struct_compt_expr(fid, scope, expr, into_tid);
            }
            return solve_builtin_compt_expr(fid, scope, expr, std::nullopt);
        }

        // guard against non-builtins
        if (!into_type.holds<TypeBuiltin>()) {
            context.emplace_diagnostic(into_type.span, diag_code::type_is_not_resolvable_at_compt,
                                       diag_type::error);
            return OptId<ExecId>{};
        }
        builtin_type into_builtin_type = into_type.as<TypeBuiltin>().type;
        return solve_builtin_compt_expr(fid, scope, expr, into_builtin_type);
    }

    [[nodiscard]] OptId<ExecId> solve_builtin_compt_expr(FileId fid, NamedOrAnonScopeId scope,
                                                         const ast_expr_t* expr,
                                                         std::optional<builtin_type> into_builtin) {
        auto emplace_e = [this, fid, expr](ExecValue val) {
            return context.register_exec(
                context, val, Span(fid, context.ast(fid).buffer(), expr->first, expr->last), true);
        };

        auto visit_def
            = [this](DefId did) { return context.def(def_visitor.visit_as_dependent(did)); };
        std::optional<ExecExprComptConstant> maybe_value;
        switch (expr->type) {
        case AST_EXPR_ID: {
            Span id_span{fid, context.ast(fid).buffer(), expr->expr.id.slice.start[0],
                         expr->expr.id.slice.start[expr->expr.id.slice.len - 1]};
            auto maybe_def = context.look_up_scoped_variable(
                scope, context.symbol_slice(expr->expr.id.slice), id_span);
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
                    if (!def.as<DefVariable>().compt_value.has_value()) {
                        return std::nullopt; // this is already malformed (already been reported, so
                                             // just return none)
                    }
                    auto exec = context.exec(def.as<DefVariable>().compt_value.as_id());
                    maybe_value = exec.template as<ExecExprComptConstant>();
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
            case TOK_INT_LIT: {
                maybe_value = ExecExprComptConstant{tkn->val.signed_integral};
            } break;
            case TOK_UINT_LIT: {
                maybe_value = ExecExprComptConstant{tkn->val.unsigned_integral};
                auto maybe_signed = maybe_value->try_safe_convert_to(builtin_type::i64);
                if (maybe_signed.has_value()) {
                    maybe_value = maybe_signed;
                }
                break;
            }
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
            bool cooked = false;
            ComptBinaryOp maybe_bin_op{expr->expr.binary.op};
            if (maybe_bin_op.holds<InvalidOp>()) {
                cooked = true;
            } else if (maybe_bin_op.holds<assign_op>()) {
                context.emplace_diagnostic(
                    Span(fid, context.ast(fid).buffer(), expr->expr.binary.op),
                    diag_code::assignment_not_permitted_in_compt_expr, diag_type::error);
                cooked = true;
            } else if (maybe_bin_op.holds<is_as_op>()) {
                OptId<ExecId> lhs = solve_builtin_compt_expr(
                    fid, scope, expr->expr.binary.lhs,
                    std::nullopt); // since we don't care about the type yet
                if (!lhs.has_value()) {
                    cooked = true;
                }
                if (maybe_bin_op.as<is_as_op>() == is_as_op::is) {
                    context.emplace_diagnostic(
                        Span(fid, context.ast(fid).buffer(), expr->expr.binary.op),
                        diag_code::is_operator_requires_run_time_values, diag_type::error);
                    cooked = true;
                } else if (lhs.has_value()) {
                    assert(maybe_bin_op.as<is_as_op>() == is_as_op::as);
                    return solve_compt_cast(fid, scope, lhs.as_id(), expr->expr.binary.rhs);
                }
            } else if (!cooked) {
                OptId<ExecId> lhs
                    = solve_builtin_compt_expr(fid, scope, expr->expr.binary.lhs, std::nullopt);
                if (!lhs.has_value()) {
                    cooked = true;
                }
                OptId<ExecId> rhs
                    = solve_builtin_compt_expr(fid, scope, expr->expr.binary.rhs, std::nullopt);
                if (!rhs.has_value()) {
                    cooked = true;
                }
                if (cooked) {
                    return std::nullopt; // already poisoned
                }
                assert(maybe_bin_op.holds<binary_op>());
                // get res of binary op like +, -, etc.
                auto maybe_eid
                    = solve_binary_compt_exec(fid, scope, lhs.as_id(), maybe_bin_op.as<binary_op>(),
                                              rhs.as_id(), into_builtin);

                if (maybe_eid.has_value()) {
                    auto exec = context.exec(maybe_eid.as_id());
                    if (!exec.template holds<ExecExprComptConstant>()) {
                        cooked = true;
                    } else {
                        maybe_value = exec.template as<ExecExprComptConstant>();
                        // std::cout << "result of binary op: " << maybe_value.value().to_string()
                        //          << '\n'; // debug
                    }
                } else {
                    cooked = true;
                }
            }
            if (cooked) {
                return std::nullopt; // cooked / poisoned already, so just return since error
                                     // should've already been reported
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
            if (expr->expr.unary.op->type == TOK_BOOL_NOT || expr->expr.unary.op->type == TOK_PLUS
                || expr->expr.unary.op->type == TOK_MINUS) {
                auto inner
                    = solve_builtin_compt_expr(fid, scope, expr->expr.unary.expr, into_builtin);
            }
            // TODO, handle bool not, plus, and minus here
            break;
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
            return std::nullopt;
        }
        if (maybe_value.has_value()) {
            if (!into_builtin.has_value()) {
                return emplace_e(maybe_value.value());
            }

            auto maybe_converted = maybe_value.value().try_safe_convert_to(into_builtin.value());

#ifdef DEBUG_BUILD

            if (!maybe_converted.has_value()) {

                std::cout << "failed to convert from "
                          << builtin_type_to_cstr(maybe_value.value().type_builtin()) << " into "
                          << builtin_type_to_cstr(into_builtin.value())
                          << " with failed value: " << maybe_value.value().to_string() << '\n';
            }
#endif
            // TODO give str cast hints here!

            if (!maybe_converted.has_value()) {
                auto from_builtin = maybe_value.value().type_builtin();
                TypeId from = context.emplace_type(TypeBuiltin{.type = from_builtin},
                                                   Span::generated(), false);
                TypeId to = context.emplace_type(TypeBuiltin{.type = into_builtin.value()},
                                                 Span::generated(), false);
                context.emplace_diagnostic(
                    Span(fid, context.ast(fid).buffer(), expr->first, expr->last),
                    diag_code::cannot_convert_value_of_type, diag_type::error,
                    DiagnosticTypeToType{.from = from, .to = to}, DiagnosticNoOtherInfo{});

                return OptId<ExecId>{};
            }
#ifdef DEBUG_BUILD
            if (!maybe_converted.value().matches_type(into_builtin.value())) {
                std::cout << builtin_type_to_cstr(maybe_converted.value().type_builtin())
                          << '\n';                                               // debug
                std::cout << builtin_type_to_cstr(into_builtin.value()) << '\n'; // debug
            }
#endif
            assert(maybe_converted.value().matches_type(into_builtin.value()));
            return emplace_e(maybe_converted.value());
        }
        context.emplace_diagnostic(Span(fid, context.ast(fid).buffer(), expr->first, expr->last),
                                   diag_code::cannot_resolve_at_compt, diag_type::error);
        // as to not duplicate compt errors from bubbling up
        return std::nullopt;
    }

    /**
     * solve a struct's value at compile-time, this essentially attempts a canonicalization down to
     * a struct-init eexpression where each field is evaluatable at compile-time
     */
    [[nodiscard]] OptId<ExecId> solve_struct_compt_expr(FileId fid, NamedOrAnonScopeId scope,
                                                        const ast_expr_t* expr, TypeId into_tid) {
        auto emplace_e = [this, fid, expr](ExecValue val) {
            return context.register_exec(
                context, val, Span(fid, context.ast(fid).buffer(), expr->first, expr->last), true);
        };

        auto visit_def
            = [this](DefId did) { return context.def(def_visitor.visit_as_dependent(did)); };

        auto expr_span = Span(fid, context.ast(fid).buffer(), expr->first, expr->last);

        switch (expr->type) {
        case AST_EXPR_ID: {
            Span id_span{fid, context.ast(fid).buffer(), expr->expr.id.slice.start[0],
                         expr->expr.id.slice.start[expr->expr.id.slice.len - 1]};
            auto maybe_def = context.look_up_scoped_variable(
                scope, context.symbol_slice(expr->expr.id.slice), id_span);
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
                    DiagnosticTypeAfterMessage{.tid = into_tid}, DiagnosticNoOtherInfo{});
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
                expr_span, diag_code::cannot_convert_value_of_type, diag_type::error,
                DiagnosticTypeToType{.from = def_variable.type, .to = into_tid},
                DiagnosticNoOtherInfo{});
            return std::nullopt;
        }
        case AST_EXPR_STRUCT_INIT: {

            auto id_slice = expr->expr.struct_init.id;
            auto sid_slice = context.symbol_slice(expr->expr.struct_init.id);
            Span id_span{fid, context.ast(fid).buffer(), expr->expr.struct_init.id.start[0],
                         expr->expr.struct_init.id.start[expr->expr.id.slice.len - 1]};
            OptId<DefId> maybe_def_of_struct
                = context.look_up_scoped_type(scope, sid_slice, id_span);

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

                if (member.holds<DefUnevaluated>()) {
                    continue; // must be poisoned
                }

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
            // type check before returning here! but only if the into type isn't var!
            if (!context.type(into_tid).template holds<TypeVar>()) {
                if (auto into_did = context.type(into_tid).template as<TypeStructure>().definition;
                    did != into_did) {
                    context.emplace_diagnostic(
                        expr_span, diag_code::cannot_convert_value_of_type, diag_type::error,
                        DiagnosticTypeToType{
                            .from = context.emplace_type(
                                TypeStructure{.definition = did},
                                Span(fid, context.ast(fid).buffer(),
                                     expr->expr.struct_init.id.start[0],
                                     expr->expr.struct_init.id
                                         .start[expr->expr.struct_init.id.len - 1]),
                                false),
                            .to = into_tid},
                        DiagnosticNoOtherInfo{});
                    return std::nullopt;
                }
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
                                   diag_type::error, DiagnosticTypeAfterMessage{into_tid},
                                   DiagnosticNoOtherInfo{});
        return std::nullopt;
    }

  private:
    [[nodiscard]] OptId<ExecId> solve_compt_cast(FileId fid, NamedOrAnonScopeId scope, ExecId eid,
                                                 const ast_expr_t* into_expr) {
        if (into_expr->type != AST_EXPR_TYPE) {
            auto span = Span{fid, context.ast(fid).buffer(), into_expr->first, into_expr->last};
            auto d0 = context.emplace_diagnostic(span, diag_code::invalid_cast, diag_type::error);
            auto d1 = context.emplace_diagnostic(
                span, diag_code::parentheses_should_be_used_for_chained_casts, diag_type::note,
                DiagnosticNoOtherInfo{});
            context.set_next_diagnostic(d0, d1);
            return std::nullopt;
        }
        const ast_type_t* ast_type = into_expr->expr.type_expr.type;
        auto maybe_tid = resolve_type(fid, scope, ast_type);
        if (!maybe_tid.has_value()) {
            // error already report in type resolution, so just return none
            return std::nullopt;
        }
        TypeId tid = maybe_tid.as_id();
        const Exec& exec = context.exec(eid);
        const Type& type = context.type(tid);
        if (!exec.holds<ExecExprComptConstant>() || !type.holds<TypeBuiltin>()) {
            context.emplace_diagnostic_with_message_value(
                exec.span, diag_code::cannot_convert_expression_to_type, diag_type::error,
                DiagnosticTypeAfterMessage{.tid = tid});
            return std::nullopt;
        }
        auto into_builtin = type.as<TypeBuiltin>().type;
        auto from_constant = exec.as<ExecExprComptConstant>();

        // allow string casts by converting to symbol id
        std::optional<ExecExprComptConstant> maybe_converted
            = (into_builtin == builtin_type::str)
                  ? ExecExprComptConstant{from_constant.to_symbol_id(context)}
                  : from_constant.try_safe_convert_to(into_builtin);
        if (!maybe_converted.has_value()) {
            context.emplace_diagnostic_with_message_value(
                exec.span, diag_code::cannot_convert_expression_to_type, diag_type::error,
                DiagnosticTypeAfterMessage{.tid = tid});
            ExecExprComptConstant eecc = exec.as<ExecExprComptConstant>();
            context.emplace_diagnostic_with_message_value(
                exec.span, diag_code::guaranteed_narrowing_of_compt_value, diag_type::note,
                DiagnosticSymbolAfterMessage(eecc.to_symbol_id(context)));
            return std::nullopt;
        }
        return context.emplace_exec(maybe_converted.value(), exec.span, /*compt*/ true);
    }

    [[nodiscard]] OptId<ExecId> solve_binary_compt_exec(FileId fid, NamedOrAnonScopeId scope,
                                                        ExecId lhs_eid, binary_op op,
                                                        ExecId rhs_eid,
                                                        std::optional<builtin_type> into_builtin) {
        const Exec& lhs_exec = context.exec(lhs_eid);
        const Exec& rhs_exec = context.exec(rhs_eid);

        auto handle_invalid_operand = [this](const Exec& exec) {
            context.emplace_diagnostic(exec.span, diag_code::invalid_operand_for_binary_expression,
                                       diag_type::error);
        };

        bool cooked = false;
        if (!lhs_exec.holds<ExecExprComptConstant>()) {
            handle_invalid_operand(lhs_exec);
            cooked = true;
        }
        if (!rhs_exec.holds<ExecExprComptConstant>()) {
            handle_invalid_operand(rhs_exec);
            cooked = true;
        }
        if (cooked) {
            return std::nullopt;
        }
        switch (op) {
        case binary_op::plus:
        case binary_op::minus:
        case binary_op::multiply:
        case binary_op::divide:
        case binary_op::modulo:
            return handle_binary_arithmetic(fid, lhs_exec, op, rhs_exec);
        case binary_op::bit_or:
        case binary_op::bit_and:
        case binary_op::bit_xor:
            return handle_binary_bitwise(fid, lhs_exec, op, rhs_exec);
        case binary_op::greater_than:
        case binary_op::less_than:
        case binary_op::greater_than_or_equal:
        case binary_op::less_than_or_equal:
        case binary_op::bool_equal:
        case binary_op::bool_not_equal:
            return handle_binary_comparision(fid, lhs_exec, op, rhs_exec);
        case binary_op::left_bitshift:
        case binary_op::right_shift_logical:
        case binary_op::right_shift_arithmetic:
            return handle_binary_shift(fid, lhs_exec, op, rhs_exec);
        case binary_op::bool_or:
        case binary_op::bool_and:
            return handle_binary_bool_conj_disj(fid, lhs_exec, op, rhs_exec);
        }
        return std::nullopt;
    }

    [[nodiscard]] OptId<TypeId> resolve_type(FileId fid, NamedOrAnonScopeId scope,
                                             const ast_type_t* type);

    [[nodiscard]] bool guard_incompatible_types(FileId fid, const Exec& lhs, const Exec& rhs,
                                                ExecExprComptConstant lhs_val,
                                                ExecExprComptConstant rhs_val) {

        if (!lhs_val.holds_same_variant_type(rhs_val)) {
            auto lhs_tid = context.emplace_type(TypeBuiltin{.type = lhs_val.type_builtin()},
                                                lhs.span, false);
            auto rhs_tid = context.emplace_type(TypeBuiltin{.type = rhs_val.type_builtin()},
                                                rhs.span, false);
            context.emplace_diagnostic_with_message_value(
                Span::combine(lhs.span, rhs.span),
                diag_code::incompatible_types_for_binary_expression, diag_type::error,
                DiagnosticTypeAndType{.lhs_tid = lhs_tid, .rhs_tid = rhs_tid});
            return true;
        }
        return false;
    }

    [[nodiscard]] bool guard_op_not_viable_for_types(FileId fid, const Exec& lhs, const Exec& rhs,
                                                     ExecExprComptConstant lhs_val, binary_op op,
                                                     ExecExprComptConstant rhs_val) {
        if (!lhs_val.has_binary_op(op) || !rhs_val.has_binary_op(op)) {
            auto lhs_tid = context.emplace_type(TypeBuiltin{.type = lhs_val.type_builtin()},
                                                lhs.span, false);
            auto rhs_tid = context.emplace_type(TypeBuiltin{.type = rhs_val.type_builtin()},
                                                rhs.span, false);
            context.emplace_diagnostic_with_message_value(
                Span::combine(lhs.span, rhs.span),
                diag_code::incompatible_types_for_binary_operator, diag_type::error,
                DiagnosticTypeAndTypeForBinaryOp{.lhs_tid = lhs_tid, .rhs_tid = rhs_tid, .op = op});
            return true;
        }
        return false;
    }

    void guard_try_converge_types(ExecExprComptConstant& lhs_val, binary_op op,
                                  ExecExprComptConstant& rhs_val) {
        // try to safely convert (more ergonomic for literals and guranteed safe conversions)
        if (!lhs_val.holds_same_variant_type(rhs_val)) {
            const bool use_lhs_type = lhs_val.has_binary_op(op);
            const bool use_rhs_type = rhs_val.has_binary_op(op);
            // prefer lhs
            if (use_lhs_type) {
                auto maybe_converted_rhs = rhs_val.try_safe_convert_to(lhs_val.type_builtin());
                rhs_val = (maybe_converted_rhs.has_value()) ? maybe_converted_rhs.value() : rhs_val;
            } else if (use_rhs_type) {
                auto maybe_converted_lhs = lhs_val.try_safe_convert_to(rhs_val.type_builtin());
                lhs_val = (maybe_converted_lhs.has_value()) ? maybe_converted_lhs.value() : lhs_val;
            }
        }
    }

    [[nodiscard]] OptId<ExecId> handle_binary_arithmetic(FileId fid, const Exec& lhs, binary_op op,
                                                         const Exec& rhs) {

        auto lhs_val = lhs.as<ExecExprComptConstant>();
        auto rhs_val = rhs.as<ExecExprComptConstant>();

        // converge types, if possible
        guard_try_converge_types(/* & */ lhs_val, op, /* & */ rhs_val);

        if (guard_incompatible_types(fid, lhs, rhs, lhs_val, rhs_val)) {
            return std::nullopt;
        }

        if (guard_op_not_viable_for_types(fid, lhs, rhs, lhs_val, op, rhs_val)) {
            return std::nullopt;
        }

        std::optional<ExecExprComptConstant> maybe_value{};

        auto guard_div_by_zero = [&]() -> bool {
            if (rhs_val.equals_zero()) {
                context.emplace_diagnostic(
                    rhs.span, diag_code::dividing_by_zero_at_compt_is_illegal, diag_type::error);
                return true;
            }
            return false;
        };

        switch (op) {
        case binary_op::plus:
            maybe_value = ExecExprComptConstant::plus(context, lhs_val, rhs_val);
            break;
        case binary_op::minus:
            maybe_value = ExecExprComptConstant::minus(lhs_val, rhs_val);
            break;
        case binary_op::multiply:
            maybe_value = ExecExprComptConstant::multiply(lhs_val, rhs_val);
            break;

            // for div and mod guard div by zero so we don't crash the interpreter
        case binary_op::divide:
            maybe_value = (guard_div_by_zero()) ? std::nullopt
                                                : ExecExprComptConstant::divide(lhs_val, rhs_val);
            break;
        case binary_op::modulo:
            maybe_value
                = guard_div_by_zero() ? std::nullopt : ExecExprComptConstant::mod(lhs_val, rhs_val);
            break;
        default:
            std::unreachable();
            break;
        }
        if (!maybe_value.has_value()) {
            return std::nullopt;
        }
        // all good so exec up
        return context.emplace_exec(maybe_value.value(), Span::combine(lhs.span, rhs.span),
                                    /*compt*/ true);
    }

    [[nodiscard]] OptId<ExecId> handle_binary_bitwise(FileId fid, const Exec& lhs, binary_op op,
                                                      const Exec& rhs) {
        // TODO
        return std::nullopt;
    }

    [[nodiscard]] OptId<ExecId> handle_binary_comparision(FileId fid, const Exec& lhs, binary_op op,
                                                          const Exec& rhs) {
        // TODO
        return std::nullopt;
    }

    [[nodiscard]] OptId<ExecId> handle_binary_shift(FileId fid, const Exec& lhs, binary_op op,
                                                    const Exec& rhs) {
        // TODO
        return std::nullopt;
    }
    [[nodiscard]] OptId<ExecId> handle_binary_bool_conj_disj(FileId fid, const Exec& lhs,
                                                             binary_op op, const Exec& rhs) {
        // TODO
        return std::nullopt;
    }
};
} // namespace hir
#endif
