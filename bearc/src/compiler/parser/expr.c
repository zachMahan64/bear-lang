#include "compiler/ast/expr.h"
#include "compiler/parser/expr.h"
#include "compiler/token.h"
#include <string.h>

token_ptr_slice_t parser_freeze_token_handle_slice(parser_t* p, vector_t* vec) {
    token_ptr_slice_t slice = {
        .start = (token_t**)arena_alloc(p->arena, vec->size * vec->elem_size),
        .len = vec->size,
    };
    memcpy((void*)slice.start, vec->data, vec->size * vec->elem_size);
    vector_destroy(vec);
    return slice;
}
