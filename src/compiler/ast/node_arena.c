// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "compiler/ast/node_arena.h"
#include "compiler/ast/node.h"
#include "containers/arena.h"
#include "containers/strimap.h"
#include <stdarg.h>
#include <stddef.h>

#define NODE_ARENA_EXPECTED_NUM_CHILDREN 4 // expected num
ast_node_arena_t ast_node_arena_create_from_token_vec(const vector_t* vec) {
    ast_node_arena_t node_arena;
    const size_t arena_chunk_size =
        (vec->size * sizeof(ast_node_t)) +
        (8L * NODE_ARENA_EXPECTED_NUM_CHILDREN); // tokens * size of a node + size of pointer *
                                                 // expected number of children
    node_arena.arena = arena_create(arena_chunk_size);
    return node_arena;
}

void ast_node_arena_destroy(ast_node_arena_t* node_arena) { arena_destroy(&node_arena->arena); }

ast_node_t* ast_node_arena_new_node(ast_node_arena_t* arena, ast_node_type_e type, token_t* token,
                                    size_t child_count) {
    const size_t size = sizeof(ast_node_t) + (sizeof(ast_node_t*) * child_count);

    ast_node_t* node = (ast_node_t*)arena_alloc(&arena->arena, size);
    if (node == NULL) {
        return NULL;
    }
    node->type = type;
    node->token = token;
    node->child_count = child_count;

    return node;
}

ast_node_t* ast_node_arena_new_node_with_children(ast_node_arena_t* arena, ast_node_type_e type,
                                                  token_t* token, size_t child_count, ...) {
    va_list args;

    // init node
    const size_t size = sizeof(ast_node_t) + (sizeof(ast_node_t*) * child_count);
    ast_node_t* node = (ast_node_t*)arena_alloc(&arena->arena, size);
    if (node == NULL) {
        return NULL;
    }
    node->type = type;
    node->token = token;
    node->child_count = child_count;

    // set children using va_list
    va_start(args, child_count);
    for (size_t i = 0; i < child_count; i++) {
        node->children[i] = va_arg(args, ast_node_t*);
    }
    va_end(args);

    return node;
}
