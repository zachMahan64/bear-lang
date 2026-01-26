//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef AST_AST_H
#define AST_AST_H

#include "compiler/ast/stmt.h"
#include "utils/file_io.h"
#include "utils/vector.h"
typedef struct {
    src_buffer_t src_buffer;
    vector_t tokens;
    arena_t arena;
    /// root ast node
    ast_stmt_t* file_stmt_root_node;
} br_ast_t;

br_ast_t ast_create_from_file(const char* file_name);
br_ast_t ast_destroy(br_ast_t* ast);

#endif
