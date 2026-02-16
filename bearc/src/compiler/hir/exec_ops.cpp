//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/exec_ops.hpp"
#include "compiler/token.h"
#include <assert.h>

namespace hir {
binary_op token_to_binary_op(token_t* tkn) {
    switch (tkn->type) {
    case TOK_PLUS:
        return binary_op::PLUS;
    case TOK_MINUS:
        return binary_op::MINUS;
    case TOK_STAR:
        return binary_op::MULTIPLY;
    case TOK_MODULO:
        return binary_op::MODULO;
    case TOK_BAR:
        return binary_op::BIT_OR;
    case TOK_AMPER:
        return binary_op::BIT_AND;
    case TOK_GT:
        return binary_op::GT;
    case TOK_LT:
        return binary_op::LT;
    case TOK_GE:
        return binary_op::GE;
    case TOK_LE:
        return binary_op::LE;
    case TOK_LSH:
        return binary_op::LSH;
    case TOK_RSHL:
        return binary_op::RSHL;
    case TOK_RSHA:
        return binary_op::RSHA;
    case TOK_BOOL_OR:
        return binary_op::BOOL_OR;
    case TOK_BOOL_AND:
        return binary_op::BOOL_AND;
    case TOK_BOOL_EQ:
        return binary_op::BOOL_EQ;
    case TOK_NE:
        return binary_op::BOOL_NE;

    default:
        assert(false && "invalid binary op token\n");
        break;
    }
}
unary_op token_to_unary_op(token_t* tkn) {
    switch (tkn->type) {
    case TOK_INC:
        return unary_op::INC;
    case TOK_DEC:
        return unary_op::DEC;
    case TOK_PLUS:
        return unary_op::PLUS;
    case TOK_MINUS:
        return unary_op::MINUS;
    case TOK_BOOL_NOT:
        return unary_op::BOOL_NOT;
    default:
        assert(false && "invalid unary op token\n");
        break;
    }
}
} // namespace hir
