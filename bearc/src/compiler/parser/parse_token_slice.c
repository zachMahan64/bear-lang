//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/parser/parse_token_slice.h"
#include "compiler/parser/parser.h"
#include "compiler/token.h"
#include "utils/vector.h"
#include <string.h>

token_ptr_slice_t parser_freeze_token_ptr_slice(parser_t* p, vector_t* vec) {
    token_ptr_slice_t slice = {
        .start = (token_t**)arena_alloc(p->arena, vec->size * vec->elem_size),
        .len = vec->size,
    };
    memcpy((void*)slice.start, vec->data, vec->size * vec->elem_size);
    vector_destroy(vec);
    return slice;
}
