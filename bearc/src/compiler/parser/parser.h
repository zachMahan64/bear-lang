// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_PARSER_H
#define COMPILER_PARSER_H
#include "utils/arena.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "utils/vector.h"
#include <stdint.h>

/**
 * primary parser structure
 * tracks a position ptr along a vector of token_t
 * -
 */
typedef struct {
    vector_t tokens;
    size_t pos;
    arena_t arena;
} parser_t;

parser_t parser_create(vector_t* tokens, arena_t* arena);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !COMPILER_PARSER_H
