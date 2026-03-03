//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/exec.hpp"
#include "compiler/hir/context.hpp"
namespace hir {
bool Exec::is_equivalent(const Context& ctx, ExecId eid1, ExecId eid2) {
    // same ExecId always means equivalent
    if (eid1 == eid2) {
        return true;
    }
    const Exec& e1 = ctx.exec(eid1);
    const Exec& e2 = ctx.exec(eid2);
    const bool both_compt = e1.compt && e2.compt;
    if (!both_compt) {
        return false;
    }
    if (e1.holds_same<ExecExprLiteral>(e2)) {
        const ExecExprLiteral& lit1 = e1.as<ExecExprLiteral>();
        const ExecExprLiteral& lit2 = e2.as<ExecExprLiteral>();
        return lit1.variant_value_equals(lit2);
    }
    return false;
}
} // namespace hir
