//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_PARSER_RULES

#include "compiler/ast/expr.h"
#include "compiler/parser/parser.h"
#include <stdint.h>

/// defines the associativity of an operator
typedef enum {
    LEFT_TO_RIGHT,
    RIGHT_TO_LEFT,
} associativity_e;

associativity_e associativity_of(uint32_t precedence);

// find the precendence an operator based on token type
uint32_t prec_binary(token_type_e type);

#endif // !COMPILER_PARSER_RULES
