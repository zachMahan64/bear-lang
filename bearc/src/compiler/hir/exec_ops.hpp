//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_EXEC_OPS_HPP
#define COMPILER_HIR_EXEC_OPS_HPP
#include <cstdint>
#include <optional>

typedef struct token token_t;

namespace hir {
enum class binary_op : uint8_t {
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    MODULO,
    BIT_OR,
    BIT_AND,
    BIT_NOT,
    BIT_XOR,
    GT,
    LT,
    GE,
    LE,
    LSH,
    RSHL,
    RSHA,
    BOOL_OR,
    BOOL_AND,
    BOOL_EQ,
    BOOL_NE,
};
enum class unary_op : uint8_t { INC, DEC, PLUS, MINUS, BOOL_NOT };

// converts a token to standard binary ops (arithmetic and comparison only)
std::optional<binary_op> token_to_binary_op(token_t* tkn);

// converts a token to stand unary ops, boolean and arithmetic ops
std::optional<unary_op> token_to_unary_op(token_t* tkn);

} // namespace hir

#endif
