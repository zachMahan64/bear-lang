//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_EXPR_TO_EXEC_HPP
#define COMPILER_HIR_EXPR_TO_EXEC_HPP

#include "compiler/ast/expr.h"
#include "compiler/hir/def_visitor.hpp"
#include "compiler/hir/indexing.hpp"

namespace hir {

struct AstExprToExec {
    InsideBodyDefVisitor& def_visitor;
    Context& context;

    AstExprToExec(Context& ctx, InsideBodyDefVisitor& def_visitor)
        : context{ctx}, def_visitor{def_visitor} {}
    [[nodiscard]] OptId<ExecId> to_exec(FileId fid, ScopeId scope, const ast_expr_t* expr,
                                        TypeId into_tid) {
        return std::nullopt; // TODO
    }
};

} // namespace hir

#endif // !COMPILER_HIR_AST_EXPR_TO_EXEC_HPP
