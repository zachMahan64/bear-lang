// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef COMPILER_DIAGNOSTICS_SRC_VIEW_H
#define COMPILER_DIAGNOSTICS_SRC_VIEW_H
#include "compiler/token.h"
#include "containers/string_view.h"
#include "file_io.h"
/*
 * builds a string view that displays source code
 */
string_view_t get_line_string_view(const src_buffer_t* src_buffer, token_t* tkn);

// TODO add a function to display a cursor pointing to the error using string_t

#endif // !COMPILER_DIAGNOSTICS_SRC_VIEW_H
