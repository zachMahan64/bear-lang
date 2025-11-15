// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef COMPILER_PARSER_H
#define COMPILER_PARSER_H

#include "compiler/ast/node.h"
#include "compiler/token.h"
#include "containers/vector.h"
#include <stdint.h>

typedef enum {
    LEFT_TO_RIGHT,
    RIGHT_TO_LEFT,
} associativity_e;

associativity_e associativity_of(uint32_t precedence);

#define PRECENDENCE_MAP_SIZE 64
// find the precendence an operator based on token type
uint32_t precendence_of_operator(token_type_e type);

typedef struct {
    ast_node_t* head;
    const char* file_name;
} ast_t;

// build up an ast from a specified vector of tokens
ast_t parser_build_ast_from_file(const char* file_name, vector_t token_vec);

#endif // !COMPILER_PARSER_H
