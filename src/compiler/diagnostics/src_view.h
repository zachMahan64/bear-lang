// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef COMPILER_DIAGNOSTICS_SRC_VIEW_H
#define COMPILER_DIAGNOSTICS_SRC_VIEW_H
#include "compiler/token.h"
#include "containers/string_view.h"

// builds a string view that displays source code w/ a line number and cursor poiting to the
// diagnostic
string_view_t build_line_preview_string_view(src_buffer_t* src_buffer, token_t* tkn); // TODO

#endif // !COMPILER_DIAGNOSTICS_SRC_VIEW_H
