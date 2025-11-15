// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef COMPILER_PARSER_H
#define COMPILER_PARSER_H

#include "compiler/ast/node.h"
#include "compiler/token.h"
#include <stdint.h>

typedef enum {
    LEFT_TO_RIGHT,
    RIGHT_TO_LEFT,
} associativity_e;

associativity_e associativity_of(uint32_t precedence);

#define PRECENDENCE_MAP_SIZE 64
uint32_t precendence_of(token_type_e type);

typedef struct {
    ast_node_t* head;
    const char* file_name;
} ast_t;

#endif // !COMPILER_PARSER_H
