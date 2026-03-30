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
    case TOK_BIT_XOR:
        return binary_op::bit_xor;
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
const char* binary_op_to_cstr(binary_op op) {
    switch (op) {
    case binary_op::plus:
        return "+";
    case binary_op::minus:
        return "-";
    case binary_op::multiply:
        return "*";
    case binary_op::divide:
        return "/";
    case binary_op::modulo:
        return "%";
    case binary_op::bit_or:
        return "|";
    case binary_op::bit_and:
        return "&";
    case binary_op::bit_xor:
        return "^";
    case binary_op::greater_than:
        return ">";
    case binary_op::less_than:
        return "<";
    case binary_op::greater_than_or_equal:
        return ">=";
    case binary_op::less_than_or_equal:
        return "<=";
    case binary_op::left_bitshift:
        return "<<";
    case binary_op::right_shift_logical:
        return ">>";
    case binary_op::right_shift_arithmetic:
        return ">>>";
    case binary_op::bool_or:
        return "||";
    case binary_op::bool_and:
        return "&&";
    case binary_op::bool_equal:
        return "==";
    case binary_op::bool_not_equal:
        return "!=";
    }
}

const char* binary_op_to_cstr(unary_op op) {
    switch (op) {
    case unary_op::inc:
        return "++";
    case unary_op::dec:
        return "--";
    case unary_op::plus:
        return "+";
    case unary_op::minus:
        return "-";
    case unary_op::bool_not:
        return "!";
    case unary_op::bit_not:
        return "~";
    }
}

const char* is_as_op_to_cstr(is_as_op op) {
    switch (op) {
    case is_as_op::is:
        return "is";
    case is_as_op::as:
        return "as";
    }
}

} // namespace hir
