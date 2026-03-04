//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_EXEC_HPP
#define COMPILER_HIR_EXEC_HPP

#include "compiler/hir/exec_ops.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/hir/span.hpp"
#include "compiler/hir/variant_helpers.hpp"
#include <cstdint>
#include <variant>

namespace hir {

// ------ struct impls -------

struct ExecBlock {
    ScopeAnonId scope;
    IdSlice<ExecId> execs;
};

struct ExecExprStmt {
    ExecId expr;
};

struct ExecBreakStmt {};

struct ExecIfStmt {
    ExecId condition;
    ExecId block;
    OptId<ExecId> else_stmt;
};

struct ExecLoopStmt {
    ExecId block;
};

struct ExecReturnStmt {
    OptId<ExecId> return_value;
};

struct ExecYieldStmt {
    OptId<ExecId> yield_value;
};

struct ExecExprIdentifier {
    DefId identifier;
};

// SymbolId corresponds to a builtin type
using ConstantValue = std::variant<SymbolId, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t,
                                   int64_t, uint64_t, char, float, double>;

/// represents literals and true compt values
struct ExecExprComptConstant : NodeWithVariantValue<ExecExprComptConstant> {
    ConstantValue value;
};

struct ExecExprListLiteral {
    IdSlice<ExecId> elems;
};

struct ExecExprAssignMove {
    ExecId lhs;
    ExecId rhs;
};

struct ExecExprAssignEqual {
    ExecId lhs;
    ExecId rhs;
};

struct ExecExprIs {
    ExecId variant_instance;
    ExecId variant_decomp;
};

struct ExecExprMemberAccess {
    ExecId owner;
    ExecId member;
};

struct ExecExprPointerMemberAccess {
    ExecId owner;
    ExecId member;
};

struct ExecExprBinary {
    ExecId lhs;
    ExecId rhs;
    binary_op op;
};

struct ExecExprCast {
    ExecId expr;
    TypeId target;
};

struct ExecExprPreUnary {
    ExecId expr;
    unary_op op;
};

struct ExecExprPostUnary {
    ExecId expr;
    unary_op op;
};

struct ExecExprSubscript {
    ExecId base;
    ExecId index;
};

struct ExecExprFnCall {
    ExecId callee;
    IdSlice<ExecId> args;
};

struct ExecExprBorrow {
    ExecId borrowee;
};

struct ExecExprDeref {
    ExecId expr;
};

struct ExecExprStructInit {
    IdSlice<ExecId> member_inits;
    DefId struct_def;
};

struct ExecExprStructMemberInit {
    DefId field_def;
    ExecId value;
    bool move;
};

struct ExecExprClosure {
    IdSlice<DefId> params;
    IdSlice<DefId> captures;
    TypeId return_type;
    bool move;
};

struct ExecExprVariantDecomp {
    IdSlice<DefId> fields;
    DefId def;
};

struct ExecExprMatch {
    ExecId scrutinee;
    IdSlice<ExecId> branches;
};

struct ExecExprMatchBranch {
    IdSlice<ExecId> patterns;
    ExecId body;
};

// ^^^^^^ struct impls ^^^^^^^^

/// main exec variant
using ExecValue = std::variant<
    // blocks / statements
    ExecBlock, ExecExprStmt, ExecBreakStmt, ExecIfStmt, ExecLoopStmt, ExecReturnStmt, ExecYieldStmt,

    // expressions
    ExecExprIdentifier, ExecExprComptConstant, ExecExprListLiteral, ExecExprAssignMove,
    ExecExprAssignEqual, ExecExprIs, ExecExprMemberAccess, ExecExprPointerMemberAccess,
    ExecExprBinary, ExecExprCast, ExecExprPreUnary, ExecExprPostUnary, ExecExprSubscript,
    ExecExprFnCall, ExecExprBorrow, ExecExprDeref, ExecExprStructInit, ExecExprStructMemberInit,
    ExecExprClosure, ExecExprVariantDecomp, ExecExprMatch, ExecExprMatchBranch>;

/// main exec structure, corresponds to an hir::ExecId
struct Exec : NodeWithVariantValue<Exec> {
    using id_type = ExecId;
    using value_type = ExecValue;
    ExecValue value;
    const Span span;
    bool compt;
    Exec(Context& ctx, ExecValue value, Span span, bool compt);
    static bool is_equivalent(const Context& ctx, ExecId eid1, ExecId eid2);

  private:
    bool can_be_compt(const Context& ctx);
};

} // namespace hir

#endif
