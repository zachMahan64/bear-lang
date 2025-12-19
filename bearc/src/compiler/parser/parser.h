//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_PARSER_H
#define COMPILER_PARSER_H
#include "compiler/diagnostics/error_list.h"
#include "utils/arena.h"
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "utils/vector.h"
#include <stdint.h>

/**
 * primary parser structure
 * tracks a position ptr along a vector of token_t
 * - does not own anything!
 */
typedef struct {
    vector_t* tokens;
    size_t pos;
    arena_t* arena;
    compiler_error_list_t* error_list;
    bool prev_discarded;
} parser_t;

parser_t parser_create(vector_t* tokens, arena_t* arena, compiler_error_list_t* error_list);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !COMPILER_PARSER_H
