//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/parser/parser.h"
#include "utils/arena.h"
#include <stddef.h>
parser_t parser_create(vector_t* tokens, arena_t* arena, compiler_error_list_t* error_list) {
    parser_t parser = {.tokens = tokens, .pos = 0, .arena = arena, .error_list = error_list};
    return parser;
}
