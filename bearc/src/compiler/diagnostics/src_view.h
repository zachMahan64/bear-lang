// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef COMPILER_DIAGNOSTICS_SRC_VIEW_H
#define COMPILER_DIAGNOSTICS_SRC_VIEW_H
#include "compiler/token.h"
#include "containers/string.h"
#include "containers/string_view.h"
#include "file_io.h"
/*
 * builds a string view that displays source code
 */
string_view_t get_line_string_view(const src_buffer_t* src_buffer, token_t* tkn);

/**
 * gets a cursor pointing to an error token, should be used with the string_view_t from
 * get_line_string_view
 */
string_t get_cursor_string(string_view_t line_view, token_t* tkn, const char* ansi_color);

#endif // !COMPILER_DIAGNOSTICS_SRC_VIEW_H
