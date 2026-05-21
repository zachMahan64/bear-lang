//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef BEARC_COMPILER_HIR_MATCHING_HPP
#define BEARC_COMPILER_HIR_MATCHING_HPP

#include "compiler/ast/expr.h"
#include "compiler/hir/expr_solver.hpp"
#include "compiler/hir/indexing.hpp"

namespace hir {

/// validates the branches of a match expression
/// returns true when match is NOT valid
template <IsExprSolver S>
bool validate_match_branches_for_variant(S& solver, DefId variant_did, ast_expr_t* match_expr,
                                         ScopeId scope, FileId fid) {
    assert(match_expr->type == AST_EXPR_MATCH);
    // TODO
    return false;
}

} // namespace hir

#endif // !BEARC_COMPILER_HIR_MATCHING_HPP
