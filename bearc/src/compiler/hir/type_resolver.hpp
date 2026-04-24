//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_TYPE_RESOLVER_HPP
#define COMPILER_HIR_TYPE_RESOLVER_HPP

#include "compiler/ast/type.h"
#include "compiler/hir/compt_expr_solver.hpp"
#include "compiler/hir/context.hpp"
#include "compiler/hir/def_visitor.hpp"
#include "compiler/hir/diagnostic.hpp"
#include "compiler/token.h"
#include <optional>

namespace hir {

template <IsDefVisitor V> class TypeResolver {
    V& def_visitor;
    Context& context;

    [[nodiscard]] OptId<TypeId> type_base(FileId fid, ScopeId scope, const ast_type_t* type,
                                          bool need_layout_info) {
        auto maybe_builtin = id_tkn_slice_to_maybe_builtin(type->type.base.id);

        const bool mut = type->type.base.mut;

        if (maybe_builtin.has_value()) {
            return context.emplace_type(TypeBuiltin{maybe_builtin.value()},
                                        Span(context, fid, type->first, type->last), mut);
        }

        if (type->type.base.id.len == 1 && type->type.base.id.start[0]->type == TOK_VAR) {
            return context.emplace_type(TypeVar{}, Span(context, fid, type->first, type->last),
                                        mut);
        }

        auto scoped_id_contains_var = +[](token_ptr_slice_t id_slice) -> bool {
            for (size_t i = 0; i < id_slice.len; i++) {
                if (id_slice.start[i]->type == TOK_VAR) {
                    return true;
                }
            }
            return false;
        };

        if (type->type.base.id.len > 1 && scoped_id_contains_var(type->type.base.id)) {
            context.emplace_diagnostic(Span(context, fid, type->first, type->last),
                                       diag_code::var_cannot_be_part_of_a_scoped_identifier,
                                       diag_type::error);
            return std::nullopt;
        }

        Span id_span{fid, context.ast(fid).buffer(), type->type.base.id.start[0],
                     type->type.base.id.start[type->type.base.id.len - 1]};
        OptId<DefId> maybe_type
            = context.look_up_scoped_type(scope, context.symbol_slice(type->type.base.id), id_span);

        Span span{context, fid, type->first, type->last};

        if (maybe_type.has_value()) {
            auto did = need_layout_info ? def_visitor.visit_as_dependent(maybe_type.as_id())
                                        : def_visitor.visit_as_transparent(maybe_type.as_id());

            const Def& def = context.def(did);

            if (def.holds<DefDeftype>()) {
                const Type& orig_type = context.type(def.as<DefDeftype>().type);

                // make the true type
                TypeId new_tid = context.emplace_type(orig_type.value, span,
                                                      mut); // rebind mut here

                return context.emplace_type(TypeDeftype{.true_type = new_tid, .definition = did},
                                            span, mut);
            }

            return context.emplace_type(TypeStructure{did}, span, mut);
        }

        context.emplace_diagnostic(span, diag_code::type_not_defined, diag_type::error);

        return OptId<TypeId>{};
    }

    OptId<TypeId> type_ptr_ref(FileId fid, ScopeId scope, const ast_type_t* type) {
        auto maybe_inner
            = resolve_type(fid, scope, type->type.ptr_ref.inner, false); // don't need layout info

        if (!maybe_inner.has_value()) {
            return OptId<TypeId>{};
        }

        if (type->type.ptr_ref.modifier->type == TOK_STAR) {
            return context.emplace_type(TypePtr{.inner = maybe_inner.as_id()},
                                        Span(context, fid, type->first, type->last),
                                        type->type.ptr_ref.mut);
        }

        return context.emplace_type(TypeRef{.inner = maybe_inner.as_id()},
                                    Span(context, fid, type->first, type->last),
                                    type->type.ptr_ref.mut);
    }

    OptId<TypeId> type_arr(FileId fid, ScopeId scope, const ast_type_t* type,
                           bool need_layout_info) {
        auto maybe_inner = resolve_type(fid, scope, type->type.arr.inner, need_layout_info);

        if (!maybe_inner.has_value()) {
            return std::nullopt;
        }

        auto maybe_size_exec = ComptExprSolver<V>{context, def_visitor}.solve_builtin_compt_expr(
            fid, scope, type->type.arr.size_expr, builtin_type::usize, std::nullopt);

        if (!maybe_size_exec.has_value()) {
            return OptId<TypeId>{};
        }

        const Exec& size_exec = context.exec(maybe_size_exec.as_id());

        auto size = size_exec.template as<ExecConst>().template as<uint64_t>();

        // guard zero-size
        if (size == 0) {
            context.emplace_diagnostic(size_exec.span, diag_code::array_cannot_have_size_zero,
                                       diag_type::error);
            return std::nullopt;
        }

        return context.emplace_type(TypeArr{.inner = maybe_inner.as_id(),
                                            .compt_size_expr = maybe_size_exec.as_id(),
                                            .canonical_size = size},
                                    Span(context, fid, type->first, type->last), false);
    }

    OptId<TypeId> type_slice(FileId fid, ScopeId scope, const ast_type_t* type) {
        auto maybe_inner
            = resolve_type(fid, scope, type->type.slice.inner, false); // don't need layout info

        if (!maybe_inner.has_value()) {
            return OptId<TypeId>{};
        }

        return context.emplace_type(TypeSlice{.inner = maybe_inner.as_id()},
                                    Span(context, fid, type->first, type->last),
                                    type->type.slice.mut);
    }

    OptId<TypeId> type_fn_ptr(FileId fid, ScopeId scope, const ast_type_t* type) {
        const auto* rt = type->type.fn_ptr.return_type;
        auto maybe_return_type = rt ? resolve_type(fid, scope, rt, false) : OptId<TypeId>{};

        if (!maybe_return_type.has_value()) {
            return OptId<TypeId>{};
        }

        auto return_type = maybe_return_type.as_id();

        llvm::SmallVector<TypeId> tid_vec;
        auto ast_param_type_slice = type->type.fn_ptr.param_types;

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
            Span(context, fid, type->first, type->last), type->type.fn_ptr.mut);
    }

    OptId<TypeId> type_variadic(FileId fid, ScopeId scope, const ast_type_t* type) {
        auto maybe_inner = resolve_type(fid, scope, type->type.variadic.inner, false);

        if (!maybe_inner.has_value()) {
            return OptId<TypeId>{};
        }

        return context.emplace_type(TypeVariadic{.inner = maybe_inner.as_id()},
                                    Span(context, fid, type->first, type->last), false);
    }

    OptId<TypeId> type_typeof(FileId fid, ScopeId scope, const ast_type_t* type) {

        assert(type->tag == AST_TYPE_TYPEOF);

        const ast_expr_t* expr = type->type.type_of.of_expr;

        auto maybe_tid
            = ComptExprSolver{context, def_visitor}.infer_type_from_compt_expr(fid, scope, expr);

        if (maybe_tid.empty()) {
            return std::nullopt;
        }

        // make new type as to update span
        return context.emplace_type(context.type(maybe_tid.as_id()).value,
                                    Span(context, fid, type->first, type->last),
                                    type->type.type_of.mut);
    }

    OptId<TypeId> type_decay(FileId fid, ScopeId scope, const ast_type_t* type_node) {

        assert(type_node->tag == AST_TYPE_DECAY);

        auto maybe_tid = resolve_type(fid, scope, type_node->type.decay.inner);

        if (maybe_tid.empty()) {
            return std::nullopt; // poisoned
        }

        return decay_type(maybe_tid.as_id(), Span{context, fid, type_node->first, type_node->last});
    }

  public:
    explicit TypeResolver(Context& ctx, V& def_visitor) : def_visitor{def_visitor}, context{ctx} {}

    [[nodiscard]] OptId<TypeId> resolve_type(FileId fid, ScopeId scope, const ast_type_t* type,
                                             bool need_layout_info) {
        switch (type->tag) {
        case AST_TYPE_BASE:
            return type_base(fid, scope, type, need_layout_info);

        case AST_TYPE_REF_PTR:
            return type_ptr_ref(fid, scope, type);

        case AST_TYPE_ARR:
            return type_arr(fid, scope, type, need_layout_info);

        case AST_TYPE_SLICE:
            return type_slice(fid, scope, type);

        case AST_TYPE_GENERIC:
            // TODO: attempt a generic instantiation
            break;

        case AST_TYPE_FN_PTR:
            return type_fn_ptr(fid, scope, type);

        case AST_TYPE_VARIADIC:
            return type_variadic(fid, scope, type);

        case AST_TYPE_TYPEOF:
            return type_typeof(fid, scope, type);
            break;
        case AST_TYPE_DECAY:
            return type_decay(fid, scope, type);
        case AST_TYPE_INVALID:
            break;
        }

        return OptId<TypeId>{};
    }

    [[nodiscard]] OptId<TypeId> resolve_type(FileId fid, ScopeId scope, const ast_type_t* type) {
        return resolve_type(fid, scope, type, false);
    }

    [[nodiscard]] OptId<TypeId> decay_type(TypeId tid, Span span = Span::generated()) {
        const Type& type = context.type(context.try_decay_ref(tid));

        // make new type as to update span and remove mut
        return context.emplace_type(type.value, span, false);
    }
};

} // namespace hir
#endif // !COMPILER_HIR_TYPE_RESOLVER_HPP
