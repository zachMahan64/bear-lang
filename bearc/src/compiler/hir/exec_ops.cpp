//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/exec_ops.hpp"
#include "compiler/token.h"
#include <assert.h>
#include <optional>

namespace hir {

std::optional<binary_op> token_to_binary_op(const token_t* tkn) {
    switch (tkn->type) {
    case TOK_PLUS:
        return binary_op::plus;
    case TOK_MINUS:
        return binary_op::minus;
    case TOK_STAR:
        return binary_op::multiply;
    case TOK_MODULO:
        return binary_op::modulo;
    case TOK_BAR:
        return binary_op::bit_or;
    case TOK_AMPER:
        return binary_op::bit_and;
    case TOK_GT:
        return binary_op::greater_than;
    case TOK_LT:
        return binary_op::less_than;
    case TOK_GE:
        return binary_op::greater_than_or_equal;
    case TOK_LE:
        return binary_op::less_than_or_equal;
    case TOK_LSH:
        return binary_op::left_bitshift;
    case TOK_RSHL:
        return binary_op::right_shift_logical;
    case TOK_RSHA:
        return binary_op::right_shift_arithmetic;
    case TOK_BOOL_OR:
        return binary_op::bool_or;
    case TOK_BOOL_AND:
        return binary_op::bool_and;
    case TOK_BOOL_EQ:
        return binary_op::bool_equal;
    case TOK_NE:
        return binary_op::bool_not_equal;
    default:
        break;
    }
    return std::nullopt;
}

std::optional<unary_op> token_to_unary_op(const token_t* tkn) {
    switch (tkn->type) {
    case TOK_INC:
        return unary_op::inc;
    case TOK_DEC:
        return unary_op::dec;
    case TOK_PLUS:
        return unary_op::plus;
    case TOK_MINUS:
        return unary_op::minus;
    case TOK_BOOL_NOT:
        return unary_op::bool_not;
    default:
        break;
    }
    return std::nullopt;
}
} // namespace hir
