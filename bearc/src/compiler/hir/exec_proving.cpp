//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/exec_proving.hpp"
#include "compiler/hir/context.hpp"

namespace hir {

bool equivalent_exec(const Context& ctx, ExecId eid1, ExecId eid2) {
    const Exec& other = ctx.exec(eid2);

    auto vs = Ovld{
        [&other](const ExecBlock& t) -> bool {
            // todo
        },
        [&other](const ExecExprStmt& t) -> bool {
            // todo
        },
        [&other](const ExecBreakStmt& t) -> bool {
            // todo
        },
        [&other](const ExecContinueStmt& t) -> bool {
            // todo
        },
        [&other](const ExecIfStmt& t) -> bool {
            // todo
        },
        [&other](const ExecLoopStmt& t) -> bool {
            // todo
        },
        [&other](const ExecReturnStmt& t) -> bool {
            // todo
        },
        [&other](const ExecYieldStmt& t) -> bool {
            // todo
        },
        [&other](const ExecExprUnionInit& t) -> bool {
            // todo
        },
        [&other](const ExecExprVariantInit& t) -> bool {
            // todo
        },
        [&other](const ExecExprStructInit& t) -> bool {
            // todo
        },
        [&other](const ExecExprStructMemberInit& t) -> bool {
            // todo
        },
        [&other](const ExecExprIdentifier& t) -> bool {
            // todo
        },
        [&other](const ExecExprComptConstant& t) -> bool {
            // todo
        },
        [&other](const ExecExprListLiteral& t) -> bool {
            // todo
        },
        [&other](const ExecExprAssignMove& t) -> bool {
            // todo
        },
        [&other](const ExecExprAssignEqual& t) -> bool {
            // todo
        },
        [&other](const ExecExprIs& t) -> bool {
            // todo
        },
        [&other](const ExecExprMemberAccess& t) -> bool {
            // todo
        },
        [&other](const ExecExprPointerMemberAccess& t) -> bool {
            // todo
        },
        [&other](const ExecExprBinary& t) -> bool {
            // todo
        },
        [&other](const ExecExprCast& t) -> bool {
            // todo
        },
        [&other](const ExecExprPreUnary& t) -> bool {
            // todo
        },
        [&other](const ExecExprPostUnary& t) -> bool {
            // todo
        },
        [&other](const ExecExprSubscript& t) -> bool {
            // todo
        },
        [&other](const ExecExprFnCall& t) -> bool {
            // todo
        },
        [&other](const ExecExprBorrow& t) -> bool {
            // todo
        },
        [&other](const ExecExprDeref& t) -> bool {
            // todo
        },
        [&other](const ExecExprClosure& t) -> bool {
            // todo
        },
        [&other](const ExecExprVariantDecomp& t) -> bool {
            // todo
        },
        [&other](const ExecExprMatch& t) -> bool {
            // todo
        },
        [&other](const ExecExprMatchBranch& t) -> bool {
            // todo
        },
        [&other](const ExecFnPtr& t) -> bool {
            // todo
        },
        [&other](const ExecVariantFieldInit& t) -> bool {
            // todo
        },
    };
}

bool possibly_equivalent_exec(ExecId eid1, ExecId eid2) {
    // TODO
    return equivalent_exec(eid1, eid2);
}

} // namespace hir
