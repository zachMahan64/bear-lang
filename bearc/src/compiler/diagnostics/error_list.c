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
    const compiler_error_t err
        = {.start_tkn = token, .error_code = error_code, .expected_token_type = TOK_NONE};
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
    const compiler_error_t err
        = {.start_tkn = token, .error_code = error_code, .expected_token_type = expected_tkn_type};
    vector_push_back(&list->list_vec, &err);
}

void print_error(const src_buffer_t* src_buffer, const char* start, size_t len, size_t line,
                 size_t col, const char* accent_color, const char* error_word,
                 const char* error_message, const char* context) {
    // line is zero-indexed inside of token_t, so adjust
    size_t adusted_line = line + 1;
    size_t adjusted_col = col + 1;

// setup " | <line num> strings"
// max buf of size 21 since size_t max is 18'446'744'073'709'551'615
#define LINE_NUM_BUF_SIZE 21
    char line_num_buf[LINE_NUM_BUF_SIZE] = {0};
    snprintf(line_num_buf, LINE_NUM_BUF_SIZE, "%zu", adusted_line);
    string_t line_num_str = string_create_and_reserve(LINE_NUM_BUF_SIZE + 4); // for spaces
    string_push_cstr(&line_num_str, "  ");
    string_push_cstr(&line_num_str, line_num_buf);
    string_t line_under_num_str = string_create_and_fill(string_size(&line_num_str), ' ');
#define COMP_ERR_SIDE_BAR "  |"
    string_push_cstr(&line_num_str, COMP_ERR_SIDE_BAR);
    string_push_cstr(&line_under_num_str, COMP_ERR_SIDE_BAR);

    // do printing now that we have all strings setup
    printf("%s\'%s\': at (line %zu,%zu): %s%s: %s%s%s%s\n", ansi_bold_white(),
           src_buffer->file_name, adusted_line, adjusted_col, accent_color, error_word,
           ansi_bold_white(), error_message, context, ansi_reset());

    string_view_t line_preview = get_line_string_view(src_buffer, start);

    // ADJUST for beauty's sake
    static const size_t LINE_LEN_CRIT_VAL = 32; // this is pretty long

    size_t revised_col = col;

    // shenanigans, but it's worth it ~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // this shifts the view down by appropriate factors of LINE_LEN_CRIT_VAL
    size_t len_factor = revised_col / LINE_LEN_CRIT_VAL;
    line_preview.start += LINE_LEN_CRIT_VAL * len_factor;
    line_preview.len -= LINE_LEN_CRIT_VAL * len_factor;
    revised_col -= LINE_LEN_CRIT_VAL * len_factor;
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    printf("%s %.*s\n", string_data(&line_num_str), (int)line_preview.len, line_preview.start);

    string_t cursor_string = get_cursor_string(line_preview, len, revised_col, accent_color);

    printf("%s %s\n", string_data(&line_under_num_str), string_data(&cursor_string));
    // print an extra "   |   "
    printf("%s\n", string_data(&line_under_num_str));

    // free resources
    string_destroy(&cursor_string);
    string_destroy(&line_num_str);
    string_destroy(&line_under_num_str);
}

// private helper
void compiler_error_print_err(const compiler_error_list_t* list, size_t i) {
    compiler_error_t* error = vector_at(&list->list_vec, i);

    const src_buffer_t* src_buffer = &list->src_buffer;
    const char* start = error->start_tkn->start;
    size_t len = error->start_tkn->len;
    size_t line = error->start_tkn->loc.line;
    size_t col = error->start_tkn->loc.col;
    const char* accent_color
        = is_really_note(error->error_code) ? ansi_bold_yellow() : ansi_bold_red();
    const char* error_word = is_really_note(error->error_code) ? "note" : "error";
    const char* error_message = error_message_for_code(error->error_code);
    const char* context = error_message_context_for(error);
    print_error(src_buffer, start, len, line, col, accent_color, error_word, error_message,
                context);
}

void compiler_error_list_print_all(const compiler_error_list_t* list) {
    ansi_init();
    size_t len = list->list_vec.size;
    for (size_t i = 0; i < len; i++) {
        compiler_error_print_err(list, i);
    }
}

bool compiler_error_list_empty(const compiler_error_list_t* list) {
    return list->list_vec.size == 0;
}

size_t compiler_error_list_count(const compiler_error_list_t* list) { return list->list_vec.size; }
