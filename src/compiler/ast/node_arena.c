// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "compiler/ast/node_arena.h"
#include "compiler/ast/node.h"
#include "containers/arena.h"
#include <stddef.h>

ast_node_arena_t ast_node_arena_create_from_token_vec(vector_t vec) {
    ast_node_arena_t node_arena;
    const size_t arena_chunk_size =
        vec.size * sizeof(ast_node_t) * 2; // num tokens * 2, abitrary baseline estimate
    node_arena.arena = arena_create(arena_chunk_size);
    return node_arena;
}

void ast_node_arena_destroy(ast_node_arena_t* node_arena) { arena_destroy(&node_arena->arena); }

ast_node_t* ast_node_arena_new_node(ast_node_arena_t* arena, ast_node_type_e type, token_t* token,
                                    size_t child_count) {
    const size_t size = sizeof(ast_node_t) + (sizeof(ast_node_t*) * child_count);

    ast_node_t* node = (ast_node_t*)arena_alloc(&arena->arena, size);

    return node;
}
