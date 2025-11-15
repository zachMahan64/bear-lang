// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "compiler/errors/error_list.h"
#include "containers/vector.h"
#include "file_io.h"

#define ERROR_LIST_ARENA_CHUNK_CAP 4096
#define ERROR_LIST_VEC_RESERVE_CAP 128

error_list_t error_list_create(src_buffer_t src_buffer) {
    error_list_t err_list = {.src_buffer = src_buffer,
                             .list_vec = vector_create_and_reserve(sizeof(compiler_error_t),
                                                                   ERROR_LIST_VEC_RESERVE_CAP)};
    return err_list;
}

void error_list_destroy(error_list_t* error_list) { vector_destroy(&error_list->list_vec); }

// push an error onto the error list
void error_list_push(error_list_t* list, const compiler_error_t* compiler_error) {
    vector_push_back(&list->list_vec, compiler_error);
}

// emplace an error onto the error list
void error_list_emplace(error_list_t* list, token_t* token, const char* error_msg) {
    const compiler_error_t err = {.token = token, .error_msg = error_msg};
    vector_push_back(&list->list_vec, &err);
}
