//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/diagnostics/error_list.h"
#include "compiler/diagnostics/error_codes.h"
#include "compiler/diagnostics/src_view.h"
#include "compiler/token.h"
#include "utils/ansi_codes.h"
#include "utils/file_io.h"
#include "utils/string.h"
#include "utils/string_view.h"
#include "utils/vector.h"
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
    const compiler_error_t err = {
        .start_tkn = token, .error_code = error_code, .expected_token_type = TOK_NONE};
    vector_push_back(&list->list_vec, &err);
}

void compiler_error_list_emplace_range(compiler_error_list_t* list, token_t* start, token_t* end,
                                       error_code_e error_code) {
    const compiler_error_t err = {.start_tkn = start,
                                  .end_tkn = end,
                                  .error_code = error_code,
                                  .expected_token_type = TOK_NONE};
    vector_push_back(&list->list_vec, &err);
}

void compiler_error_list_emplace_expected_token(compiler_error_list_t* list, token_t* token,
                                                error_code_e error_code,
                                                token_type_e expected_tkn_type) {
    const compiler_error_t err = {
        .start_tkn = token, .error_code = error_code, .expected_token_type = expected_tkn_type};
    vector_push_back(&list->list_vec, &err);
}

// private helper
void compiler_error_print_err(const compiler_error_list_t* list, size_t i) {
    compiler_error_t* err = vector_at(&list->list_vec, i);

    // line is zero-indexed inside of token_t, so adjust
    size_t line = err->start_tkn->loc.line + 1;
    size_t col = err->start_tkn->loc.col + 1;

// setup " | <line num> strings"
// max buf of size 21 since size_t max is 18'446'744'073'709'551'615
#define LINE_NUM_BUF_SIZE 21
    char line_num_buf[LINE_NUM_BUF_SIZE] = {0};
    snprintf(line_num_buf, LINE_NUM_BUF_SIZE, "%zu", line);
    string_t line_num_str = string_create_and_reserve(LINE_NUM_BUF_SIZE + 4); // for spaces
    string_push_cstr(&line_num_str, "  ");
    string_push_cstr(&line_num_str, line_num_buf);
    string_t line_under_num_str = string_create_and_fill(string_size(&line_num_str), ' ');
    string_push_cstr(&line_num_str, "  |");
    string_push_cstr(&line_under_num_str, "  |");

    // do printing now that we have all strings setup
    printf(ANSI_BOLD "\'%s\': at (line %zu,%zu): " ANSI_RED_FG "error: " ANSI_RESET ANSI_BOLD
                     "%s%s" ANSI_RESET "\n",
           list->src_buffer.file_name, line, col, error_message_for_code(err->error_code),
           error_message_context_for(err));

    string_view_t line_preview = get_line_string_view(&list->src_buffer, err->start_tkn);

// ADJUST for beauty's sake
#define LINE_LEN_CRIT_VAL 32 // this is pretty long

    token_t revised_tkn = *err->start_tkn;

    // shenanigans, but it's worth it ~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // this shifts the view down by appropriate factors of LINE_LEN_CRIT_VAL
    size_t len_factor = revised_tkn.loc.col / LINE_LEN_CRIT_VAL;
    line_preview.start += LINE_LEN_CRIT_VAL * len_factor;
    line_preview.len -= LINE_LEN_CRIT_VAL * len_factor;
    revised_tkn.start += LINE_LEN_CRIT_VAL * len_factor;
    revised_tkn.loc.col -= LINE_LEN_CRIT_VAL * len_factor;
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    printf("%s %.*s\n", string_data(&line_num_str), (int)line_preview.len, line_preview.start);

    string_t cursor_string = get_cursor_string(line_preview, &revised_tkn, ANSI_RED_FG);

    printf("%s %s\n", string_data(&line_under_num_str), string_data(&cursor_string));

    // free resources
    string_destroy(&cursor_string);
    string_destroy(&line_num_str);
    string_destroy(&line_under_num_str);
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
