// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef COMPILER_PARSER_H
#define COMPILER_PARSER_H

#include "compiler/ast/node.h"
#include "compiler/token.h"

typedef struct {
    ast_node_t* head;
    const char* file_name;
} ast_t;

#endif // !COMPILER_PARSER_H
