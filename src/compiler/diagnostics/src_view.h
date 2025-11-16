// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef COMPILER_DIAGNOSTICS_SRC_VIEW_H
#define COMPILER_DIAGNOSTICS_SRC_VIEW_H
#include "compiler/token.h"
#include "containers/string_view.h"
#include "file_io.h"
/*
 * builds a string view that displays source code w/ a line number and cursor poiting to the
 * diagnostic
 */
string_view_t get_line_string_view(const src_buffer_t* src_buffer, token_t* tkn);

#endif // !COMPILER_DIAGNOSTICS_SRC_VIEW_H
