// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "compiler/diagnostics/error_list.h"
#include "ansi_codes.h"
#include "compiler/diagnostics/error_codes.h"
#include "compiler/diagnostics/src_view.h"
#include "compiler/token.h"
#include "containers/string.h"
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

// private helper
void compiler_error_print_err(const compiler_error_list_t* list, size_t i) {
    compiler_error_t* err = vector_at(&list->list_vec, i);

    printf(ANSI_BOLD "\"%s\" on line %zu: " ANSI_RED_FG "error: " ANSI_RESET ANSI_BOLD
                     "%s" ANSI_RESET "\n",
           list->src_buffer.file_name, err->token->loc.line + 1,
           error_message_for(err->error_code)); // line is zero-indexed, so adjust

    string_view_t line_preview = get_line_string_view(&list->src_buffer, err->token);
    printf("%.*s\n", (int)line_preview.len, line_preview.start);

    string_t cursor_string = get_cursor_string(line_preview, err->token, ANSI_RED_FG);
    printf("%s\n", string_get_data(&cursor_string));
    string_destroy(&cursor_string);
}

void compiler_error_list_print_all(const compiler_error_list_t* list) {
    size_t len = list->list_vec.size;
    for (size_t i = 0; i < len; i++) {
        compiler_error_print_err(list, i);
    }
    if (len == 0) {
        return; // no errors
    }
    if (len == 1) {
        puts("1 error generated.");
        return;
    }
    printf(ANSI_BOLD "%zu errors generated.\n" ANSI_RESET, len);
}

bool compiler_error_list_empty(const compiler_error_list_t* list) {
    return list->list_vec.size == 0;
}
