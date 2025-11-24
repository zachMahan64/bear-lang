// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef COMPILER_AST_NODE_ARENA
#define COMPILER_AST_NODE_ARENA

#include "compiler/ast/node.h"
#include "containers/arena.h"
#include "containers/vector.h"
#include <stddef.h>

typedef struct {
    arena_t arena;
} ast_node_arena_t;

// creates an ast_node_arena_t based on the length of a vector of tokens
ast_node_arena_t ast_node_arena_create_from_token_vec(const vector_t* vec);

// destroy an ast_node_arena_t and clean up its resources
void ast_node_arena_destroy(ast_node_arena_t*);

// allocates and returns a pointer to a new node without setting its children
ast_node_t* ast_node_arena_new_node(ast_node_arena_t* arena, ast_node_type_e type, token_t* token,
                                    size_t child_count);

// allocates and returns a pointer to a new node with its children
ast_node_t* ast_node_arena_new_node_with_children(ast_node_arena_t* arena, ast_node_type_e type,
                                                  token_t* token, size_t child_count, ...);

#endif // !COMPILER_AST_NODE_ARENA
