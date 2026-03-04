//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/exec.hpp"
#include "compiler/hir/context.hpp"
#include "compiler/hir/diagnostic.hpp"
namespace hir {

Exec::Exec(Context& ctx, ExecValue value, Span span, bool should_be_compt)
    : value{value}, span{span} {
    bool truely_compt = can_be_compt(ctx);
    if (should_be_compt && !truely_compt) {
        ctx.emplace_diagnostic(span, diag_code::value_cannot_be_compt, diag_type::error);
    }
    this->compt = truely_compt && should_be_compt;
}

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
    if (e1.holds_same<ExecExprComptConstant>(e2)) {
        const ExecExprComptConstant& lit1 = e1.as<ExecExprComptConstant>();
        const ExecExprComptConstant& lit2 = e2.as<ExecExprComptConstant>();
        return lit1.variant_value_equals(lit2);
    }
    return false;
}

bool Exec::can_be_compt(const Context& ctx) {
    auto get_d = [&](DefId did) { return ctx.def(did); };
    auto get_e = [&](ExecId eid) { return ctx.exec(eid); };
    auto eidx_to_e = [&](IdIdx<ExecId> eid) { return ctx.exec(eid); };
    auto vs = Ovld{
        [&](const ExecBlock& t) -> bool { return false; },
        [&](const ExecExprStmt& t) -> bool { return false; },
        [&](const ExecBreakStmt& t) -> bool { return false; },
        [&](const ExecIfStmt& t) -> bool { return false; },
        [&](const ExecLoopStmt& t) -> bool { return false; },
        [&](const ExecReturnStmt& t) -> bool { return false; },
        [&](const ExecYieldStmt& t) -> bool { return false; },
        // exprs
        [&](const ExecExprIdentifier& t) -> bool { return get_d(t.identifier).compt; },
        [&](const ExecExprComptConstant& t) -> bool { return true; },
        [&](const ExecExprListLiteral& t) -> bool {
            // just check each elem
            for (auto eidx = t.elems.begin(); eidx != t.elems.end(); eidx++) {
                if (!eidx_to_e(eidx).compt) {
                    return false;
                }
            }
            return true;
        },
        [&](const ExecExprAssignMove& t) -> bool {
            return get_e(t.lhs).compt && get_e(t.rhs).compt;
        },
        [&](const ExecExprAssignEqual& t) -> bool {
            return get_e(t.lhs).compt && get_e(t.rhs).compt;
        },
        [&](const ExecExprIs& t) -> bool {
            return get_e(t.variant_instance).compt && get_e(t.variant_decomp).compt;
        },
        [&](const ExecExprMemberAccess& t) -> bool {
            return get_e(t.owner).compt && get_e(t.member).compt;
        },
        [&](const ExecExprPointerMemberAccess& t) -> bool {
            return get_e(t.owner).compt && get_e(t.member).compt;
        },
        [&](const ExecExprBinary& t) -> bool { return get_e(t.lhs).compt && get_e(t.rhs).compt; },
        [&](const ExecExprCast& t) -> bool { return get_e(t.expr).compt; },
        [&](const ExecExprPreUnary& t) -> bool { return get_e(t.expr).compt; },
        [&](const ExecExprPostUnary& t) -> bool { return get_e(t.expr).compt; },
        [&](const ExecExprSubscript& t) -> bool {
            return get_e(t.base).compt && get_e(t.index).compt;
        },
        [&](const ExecExprFnCall& t) -> bool {
            const bool callee = get_e(t.callee).compt;
            // check all args, ret false if one isn't compt
            for (auto eidx = t.args.begin(); eidx != t.args.end(); eidx++) {
                if (!eidx_to_e(eidx).compt) {
                    return false;
                }
            }
            return callee;
        },
        [&](const ExecExprBorrow& t) -> bool { return get_e(t.borrowee).compt; },
        [&](const ExecExprDeref& t) -> bool { return get_e(t.expr).compt; },
        [&](const ExecExprStructInit& t) -> bool {
            for (auto eidx = t.member_inits.begin(); eidx != t.member_inits.end(); eidx++) {
                if (!eidx_to_e(eidx).compt) {
                    return false;
                }
            }
            return true;
        },
        [&](const ExecExprStructMemberInit& t) -> bool { return get_e(t.value).compt; },
        // full compt ctrl no for now
        [&](const ExecExprClosure& t) -> bool { return false; },
        [&](const ExecExprVariantDecomp& t) -> bool { return false; },
        [&](const ExecExprMatch& t) -> bool { return false; },
        [&](const ExecExprMatchBranch& t) -> bool { return false; },
    };
    return visit(vs);
}

} // namespace hir
