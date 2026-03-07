//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_TOP_LEVEL_DEF_VISITOR_HPP
#define COMPILER_TOP_LEVEL_DEF_VISITOR_HPP

#include "compiler/ast/expr.h"
#include "compiler/hir/indexing.hpp"
#include "compiler/hir/scope.hpp"
#include "compiler/hir/type.hpp"
#include "llvm/ADT/SmallVector.h"

namespace hir {

class Context;

class TopLevelDefVisitor {

    static constexpr size_t DEF_STACK_SIZE = 512;

    Context& context;
    // for tracking the stack of defs to report circular defs
    llvm::SmallVector<DefId, DEF_STACK_SIZE> def_stack;
    bool began_resolution;

    DefId resolve_def(DefId def);
    DefId visit(DefId def);
    void report_cycle(DefId culprit);

  public:
    TopLevelDefVisitor(Context& context) : context{context}, began_resolution{false} {}
    void resolve_top_level_definitions();
    /// visit a DefId where the def being visited is depended on by the visitor
    DefId visit_as_dependent(DefId def);
    /// visit when not all info is need (i.e. just validate existence for pointers/references)
    DefId visit_as_transparent(DefId def);
};

class TopLevelConstantExprSolver {
    TopLevelDefVisitor& def_visitor;
    Context& context;

  public:
    explicit TopLevelConstantExprSolver(Context& ctx, TopLevelDefVisitor& def_visitor)
        : context{ctx}, def_visitor{def_visitor} {}
    // solves a top level compt expr (this is primarily for array sizing & builtin types for top
    // level generic instantiation with compt parameterizations)
    [[nodiscard]] OptId<ExecId> solve_compt_expr(FileId fid, NamedOrAnonScopeId scope,
                                                 const ast_expr_t* expr, TypeId into);
    [[nodiscard]] OptId<ExecId> solve_compt_expr(FileId fid, NamedOrAnonScopeId scope,
                                                 const ast_expr_t* expr, builtin_type into_builtin);
};

class TopLevelTypeResolver {
    TopLevelDefVisitor& def_visitor;
    Context& context;

    [[nodiscard]] OptId<TypeId> type_base(FileId fid, NamedOrAnonScopeId scope,
                                          const ast_type_t* type);
    [[nodiscard]] OptId<TypeId> type_ptr_ref(FileId fid, NamedOrAnonScopeId scope,
                                             const ast_type_t* type);
    [[nodiscard]] OptId<TypeId> type_arr(FileId fid, NamedOrAnonScopeId scope,
                                         const ast_type_t* type);
    [[nodiscard]] OptId<TypeId> type_slice(FileId fid, NamedOrAnonScopeId scope,
                                           const ast_type_t* type);

    [[nodiscard]] OptId<TypeId> type_fn_ptr(FileId fid, NamedOrAnonScopeId scope,
                                            const ast_type_t* type);

  public:
    explicit TopLevelTypeResolver(Context& ctx, TopLevelDefVisitor& def_visitor)
        : context{ctx}, def_visitor{def_visitor} {}
    // resolves a mentioned type at the top level
    [[nodiscard]] OptId<TypeId> resolve_type(FileId fid, NamedOrAnonScopeId scope,
                                             const ast_type_t* type);
};

} // namespace hir

#endif
