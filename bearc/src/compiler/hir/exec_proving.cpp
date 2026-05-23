//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/exec_proving.hpp"
#include "compiler/hir/context.hpp"
#include "compiler/hir/exec.hpp"
#include "compiler/hir/indexing.hpp"

namespace hir {

bool equivalent_exec(const Context& ctx, ExecId eid1, ExecId eid2) {
    const Exec& other = ctx.exec(eid2);

    auto vs = Ovld{
        [](const ExecBlock& t) -> bool { return false; },
        [](const ExecExprStmt& t) -> bool { return false; },
        [](const ExecBreakStmt& t) -> bool { return false; },
        [](const ExecContinueStmt& t) -> bool { return false; },
        [](const ExecIfStmt& t) -> bool { return false; },
        [](const ExecLoopStmt& t) -> bool { return false; },
        [](const ExecReturnStmt& t) -> bool { return false; },
        [](const ExecYieldStmt& t) -> bool { return false; },
        [&other, &ctx](const ExecExprUnionInit& t) -> bool {
            if (!other.holds<ExecExprUnionInit>()) {
                return false;
            }

            if (t.union_def_id != other.as<ExecExprUnionInit>().union_def_id) {
                return false;
            }

            if (t.active_member_idx != other.as<ExecExprUnionInit>().active_member_idx) {
                return false;
            }

            return equivalent_exec(ctx, t.member_init, other.as<ExecExprUnionInit>().member_init);
        },
        [&other, &ctx](const ExecExprVariantInit& t) -> bool {
            if (!other.holds<ExecExprVariantInit>()) {
                return false;
            }
            if (t.variant_def_id != other.as<ExecExprVariantInit>().variant_def_id) {
                return false;
            }

            if (t.active_member_idx != other.as<ExecExprVariantInit>().active_member_idx) {
                return false;
            }

            return equivalent_exec(ctx, t.payload_init,
                                   other.as<ExecExprVariantInit>().payload_init);
        },
        [&other, &ctx](const ExecExprStructInit& t) -> bool {
            if (!other.holds<ExecExprStructInit>()) {
                return false;
            }

            const auto o = other.as<ExecExprStructInit>();

            if (t.struct_def_id != o.struct_def_id) {
                return false;
            }

            // malformed guard
            if (t.member_inits.len() != o.member_inits.len()) {
                return false;
            }

            // compare each member init sequentially and just ret false if a single one disagrees
            for (HirSize i = 0; i < o.member_inits.len(); ++i) {
                if (!equivalent_exec(ctx, ctx.exec_id(o.member_inits.get(i)),
                                     ctx.exec_id(t.member_inits.get(i)))) {
                    return false;
                }
            }

            return true;
        },
        [&other, &ctx](const ExecExprStructMemberInit& t) -> bool {
            if (!other.holds<ExecExprStructMemberInit>()) {
                return false;
            }

            const auto o = other.as<ExecExprStructMemberInit>();

            if (t.field_def != o.field_def) {
                return false;
            }

            return equivalent_exec(ctx, t.value, o.value);
        },
        [](const ExecExprVariable& t) -> bool { return false; },
        [&other](const ExecExprComptConstant& t) -> bool {
            if (!other.holds<ExecExprComptConstant>()) {
                return false;
            }
            const auto o = other.as<ExecExprComptConstant>();
            if (!o.holds_same_variant_type(t)) {
                return false;
            }
            const auto res = ExecConst::equal(t, o);
            return res.has_value() && res.value().as<bool>();
        },
        [&other, &ctx](const ExecExprListLiteral& t) -> bool {
            if (!other.holds_same_variant_type(t)) {
                return false;
            }

            const auto o = other.as<ExecExprListLiteral>();

            if (o.len() != t.len()) {
                return false;
            }

            if (o.elem_type_id.has_value() != t.elem_type_id.has_value()) {
                return false;
            }

            if (o.elem_type_id.has_value() && t.elem_type_id.has_value()
                && !ctx.equivalent_type(o.elem_type_id.as_id(), t.elem_type_id.as_id())) {
                return false;
            }

            for (HirSize i = 0; i < o.len(); ++i) {
                if (equivalent_exec(ctx, ctx.exec_id(o.elems.get(i)),
                                    ctx.exec_id(t.elems.get(i)))) {
                    return false;
                }
            }
            return true;
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

bool possibly_equivalent_exec(const Context& ctx, ExecId eid1, ExecId eid2) {
    // TODO
    return equivalent_exec(ctx, eid1, eid2);
}

} // namespace hir
