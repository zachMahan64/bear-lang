//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_DEF_VISITOR_HPP
#define COMPILER_HIR_DEF_VISITOR_HPP

#include "compiler/hir/indexing.hpp"
#include "compiler/hir/scope.hpp"
#include "compiler/hir/type.hpp"
#include "llvm/ADT/SmallVector.h"
#include <concepts>

namespace hir {

class Context;

template <typename T>
concept IsDefVisitor = requires(T t, DefId def) {
    // do not alter mention state
    { t.visit_as_independent(def) } -> std::convertible_to<DefId>;
    // alter mention state (mentioned)
    { t.visit_as_dependent(def) } -> std::convertible_to<DefId>;
    // alter mention state (mentioned) and never resolve
    { t.visit_as_transparent(def) } -> std::convertible_to<DefId>;
    // alter mention state (mutated)
    { t.visit_as_mutator(def) } -> std::convertible_to<DefId>;
};

class TopLevelDefVisitor {

    static constexpr size_t DEF_STACK_SIZE = 512;

    Context& context;
    // for tracking the stack of defs to report circular defs
    llvm::SmallVector<DefId, DEF_STACK_SIZE> def_stack;
    bool began_resolution;

    DefId resolve_def(DefId did);
    DefId visit_and_resolve_if_needed(DefId def);
    void report_cycle(DefId culprit);

  public:
    TopLevelDefVisitor(Context& context) : context{context}, began_resolution{false} {}
    void resolve_top_level_definitions();
    /// visit a DefId where the def being visited is depended on by the visitor
    DefId visit_as_dependent(DefId def);

    DefId visit_as_independent(DefId def);

    DefId visit_as_mutator(DefId def);

    /// visit when not all info is need (i.e. just validate existence for pointers/references)
    [[nodiscard]] DefId visit_as_transparent(DefId def) noexcept;
};
static_assert(IsDefVisitor<TopLevelDefVisitor>);

class InsideBodyDefVisitor {
    Context& context;

  public:
    InsideBodyDefVisitor(Context& context) : context{context} {}
    DefId visit_as_dependent(DefId def);

    DefId visit_as_transparent(DefId def);

    static DefId visit_as_independent(DefId def);

    DefId visit_as_mutator(DefId def);
};
static_assert(IsDefVisitor<InsideBodyDefVisitor>);

} // namespace hir

#endif
