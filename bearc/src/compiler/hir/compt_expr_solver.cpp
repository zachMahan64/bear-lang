//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/compt_expr_solver.hpp"
#include "compiler/hir/def_visitor.hpp"

namespace hir {

// explicit instants., these should be the only ones necessary
template class ComptExprSolver<InsideBodyDefVisitor>;
template class ComptExprSolver<TopLevelDefVisitor>;
} // namespace hir
