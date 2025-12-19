//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "utils/spill_arr.h"
#include "utils/vector.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

/// makes a spill_arr_8_t of internal default capacity
spill_arr_ptr_t spill_arr_ptr_create(void) {
    spill_arr_ptr_t arr = {
        .size = 0,
        .arr_cap = SPILL_ARR_SIZE_8_T_ARR_CAP,
        // vec uninit until we need it,
        // arr uninit'd
    };
    return arr;
}

void** spill_arr_ptr_emplace(spill_arr_ptr_t* sarr) {
    // emplace into internal buffer unless size >= cap
    if (sarr->size < sarr->arr_cap) {
        ++sarr->size;
        return (sarr->data + (sarr->size - 1));
    }
    ++sarr->size;
    // critical point
    if (sarr->size == sarr->arr_cap) {
        sarr->vec = vector_create_and_reserve(sizeof(void*), SPILL_ARR_SIZE_8_T_ARR_CAP);
    }
    return (void**)vector_emplace_back(&sarr->vec);
}

void** spill_arr_ptr_at(spill_arr_ptr_t* sarr, size_t n) {
    if (n >= sarr->size) {
        return NULL;
    }
    if (n < sarr->arr_cap) {
        return (sarr->data + n);
    }
    // assume internal buff is 64
    // say we ask for idx 64, we actually get 64 - 64 = vec[0]
    assert(sarr->vec.elem_size == sizeof(void*) &&
           "[spill_arr_8_at] internal vector elem_size corrupted\n");
    return (void**)vector_at(&sarr->vec, n - sarr->arr_cap);
}

void spill_arr_ptr_destroy(spill_arr_ptr_t* sarr) {
    if (sarr->size < sarr->arr_cap) {
        return;
    }
    vector_destroy(&sarr->vec);
}
