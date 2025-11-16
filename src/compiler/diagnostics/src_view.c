// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "compiler/diagnostics/src_view.h"
#include "ansi_codes.h"
#include "compiler/token.h"
#include "containers/string.h"
#include "containers/string_view.h"
#include "file_io.h"

string_view_t get_line_string_view(const src_buffer_t* src_buffer, token_t* tkn) {
    // find start of line
    ptrdiff_t diff_to_line_start = 0;
    while (*(tkn->start - diff_to_line_start) != '\n' &&
           tkn->start - diff_to_line_start != src_buffer->data) {
        diff_to_line_start++;
    }
    diff_to_line_start--; // backtrack once because we overshot in the final increment

    // find end of line
    ptrdiff_t diff_to_line_end = 0;
    while (*(tkn->start + diff_to_line_end) != '\n' && *(tkn->start + diff_to_line_end) != '\0' &&
           tkn->start + diff_to_line_end != src_buffer->data) {
        diff_to_line_end++;
    }
    string_view_t string_view = {.start = tkn->start - diff_to_line_start,
                                 .len = diff_to_line_start + diff_to_line_end};
    return string_view;
}

string_t get_cursor_string(string_view_t line_view, token_t* tkn, const char* ansi_color) {
    string_t str = string_create_and_reserve(line_view.len + 20); // room for ansi strings
    string_push_cstring(&str, ansi_color);
    string_push_cstring(&str, ANSI_BOLD);
    for (size_t i = 0; i < line_view.len; i++) {
        if (i == tkn->loc.col) {
            string_push_char(&str, '^');
            continue;
        }
        string_push_char(&str, '~');
    }
    string_push_cstring(&str, ANSI_RESET);
    return str;
}
