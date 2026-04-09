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
#include "compiler/hir/type.hpp"
#include "compiler/hir/variant_helpers.hpp"
#include <cstdint>
#include <optional>
#include <string>
#include <variant>

namespace hir {

// ------ struct impls -------

struct ExecBlock {
    ScopeId scope;
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
using ConstantValue
    = std::variant</* 0 */ SymbolId, /* 1 */ int8_t, /*2*/ uint8_t, /*3*/ int16_t, /*4*/ uint16_t,
                   /*5*/ int32_t, /*6*/ uint32_t,
                   /*7*/ int64_t, /*8*/ uint64_t, /*9*/ char, /*10*/ float, /*11*/ double,
                   /* 12 */ std::nullptr_t, /* 13 */ bool>;

/// represents literals and true compt values
struct ExecExprComptConstant : NodeWithVariantValue<ExecExprComptConstant> {
    ConstantValue value;
    bool matches_type(builtin_type type) const {
        switch (value.index()) {
        case 0:
            return type == builtin_type::str;
        case 1:
            return type == builtin_type::i8;
        case 2:
            return type == builtin_type::u8;
        case 3:
            return type == builtin_type::i16;
        case 4:
            return type == builtin_type::u16;
        case 5:
            return type == builtin_type::i32;
        case 6:
            return type == builtin_type::u32;
        case 7:
            return type == builtin_type::i64;
        case 8:
            return type == builtin_type::u64 || type == builtin_type::usize;
        case 9:
            return type == builtin_type::charr;
        case 10:
            return type == builtin_type::f32;
        case 11:
            return type == builtin_type::f64;
        case 12:
            return type == builtin_type::nullpointer;
        case 13:
            return type == builtin_type::boolean;
        default:
            assert(false && "unconsidered builtin type");
            return false;
        }
    }
    builtin_type type_builtin() const {
        switch (value.index()) {
        case 0:
            return builtin_type::str;
        case 1:
            return builtin_type::i8;
        case 2:
            return builtin_type::u8;
        case 3:
            return builtin_type::i16;
        case 4:
            return builtin_type::u16;
        case 5:
            return builtin_type::i32;
        case 6:
            return builtin_type::u32;
        case 7:
            return builtin_type::i64;
        case 8:
            return builtin_type::u64;
        case 9:
            return builtin_type::charr;
        case 10:
            return builtin_type::f32;
        case 11:
            return builtin_type::f64;
        case 12:
            return builtin_type::nullpointer;
        case 13:
            return builtin_type::boolean;
        default:
            assert(false && "unconsidered builtin type");
            return builtin_type::voidd;
        }
    }

    // straight up string converter, use this mostly just for debugging
    std::string to_string();

    // basically a string converter but fancy
    SymbolId to_symbol_id(Context& ctx) const;

    // returns none if conversion fails, diagnostics must be reported outside of this method
    [[nodiscard]] std::optional<ExecExprComptConstant> try_safe_convert_to(builtin_type type) const;

    ExecExprComptConstant(ConstantValue constval) : value{constval} {}

    [[nodiscard]] std::optional<ExecExprComptConstant> try_up_convert_to(builtin_type type) const;
    [[nodiscard]] std::optional<ExecExprComptConstant> try_down_convert_to(builtin_type type) const;

    [[nodiscard]] bool has_binary_op(binary_op op) const;
    [[nodiscard]] bool has_unary_op(unary_op op) const;

    [[nodiscard]] bool equals_zero() const;

    using ExecConst = ExecExprComptConstant;

    [[nodiscard]] static std::optional<ExecConst> plus(Context& ctx, ExecConst lhs, ExecConst rhs);
    [[nodiscard]] static std::optional<ExecConst> minus(ExecConst lhs, ExecConst rhs);
    [[nodiscard]] static std::optional<ExecConst> multiply(ExecConst lhs, ExecConst rhs);
    [[nodiscard]] static std::optional<ExecConst> divide(ExecConst lhs, ExecConst rhs);
    [[nodiscard]] static std::optional<ExecConst> mod(ExecConst lhs, ExecConst rhs);

    [[nodiscard]] static std::optional<ExecConst> bit_and(ExecConst lhs, ExecConst rhs);
    [[nodiscard]] static std::optional<ExecConst> bit_or(ExecConst lhs, ExecConst rhs);
    [[nodiscard]] static std::optional<ExecConst> bit_xor(ExecConst lhs, ExecConst rhs);
    [[nodiscard]] static std::optional<ExecConst> bit_lsh(ExecConst lhs, ExecConst rhs);
    [[nodiscard]] static std::optional<ExecConst> bit_rsha(ExecConst lhs, ExecConst rhs);
    [[nodiscard]] static std::optional<ExecConst> bit_rshl(ExecConst lhs, ExecConst rhs);
};

using ExecConst = ExecExprComptConstant;
using i8 = int8_t;
using u8 = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;
using usize = uint64_t;
using f32 = float;
using f64 = double;

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
    Exec(Context& ctx, ExecValue value, Span span, bool should_be_compt);
    static bool is_equivalent(const Context& ctx, ExecId eid1, ExecId eid2);

  private:
    bool can_be_compt(const Context& ctx);
};

} // namespace hir

#endif
