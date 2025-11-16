// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "compiler/diagnostics/error_list.h"
#include "compiler/diagnostics/error_codes.h"
#include "compiler/token.h"
#include "containers/string_view.h"
#include "containers/vector.h"
#include "file_io.h"
#include <stddef.h>
#include <stdio.h>

#define ERROR_LIST_ARENA_CHUNK_CAP 4096
#define ERROR_LIST_VEC_RESERVE_CAP 128

compiler_error_list_t compiler_error_list_create(src_buffer_t* src_buffer) {
    compiler_error_list_t err_list = {.src_buffer = *src_buffer,
                                      .list_vec = vector_create_and_reserve(
                                          sizeof(compiler_error_t), ERROR_LIST_VEC_RESERVE_CAP)};
    return err_list;
}

void compiler_error_list_destroy(compiler_error_list_t* error_list) {
    vector_destroy(&error_list->list_vec);
}

void compiler_error_list_push(compiler_error_list_t* list, const compiler_error_t* compiler_error) {
    vector_push_back(&list->list_vec, compiler_error);
}

void compiler_error_list_emplace(compiler_error_list_t* list, token_t* token,
                                 error_code_e error_code) {
    const compiler_error_t err = {.token = token, .error_code = error_code};
    vector_push_back(&list->list_vec, &err);
}

// TODO, ANSI escape strings?

void compiler_error_list_print_all(const compiler_error_list_t* list) {
    size_t len = list->list_vec.size;
    for (size_t i = 0; i < len; i++) {
        compiler_error_t* err = vector_at(&list->list_vec, i);
        printf("[ERROR] %s in \"%s\" on line %zu \n", error_message_for(err->error_code),
               list->src_buffer.file_name,
               err->token->loc.line + 1); // line is zero-indexed, so adjust

        // TODO add line preview
    }
    if (len == 0) {
        return; // no errors
    }
    if (len == 1) {
        puts("1 error generated.");
        return;
    }
    printf("%zu errors generated.\n", len);
}
