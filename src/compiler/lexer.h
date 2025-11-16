// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef COMPILER_LEXER
#define COMPILER_LEXER

#include "compiler/errors/error_list.h"
#include "containers/vector.h"
#include "file_io.h"

#define LEXER_ESTIMATED_CHARS_PER_TOKEN 6

vector_t lexer_tokenize_src_buffer(const src_buffer_t* buf);
void find_lexer_errors(const vector_t* token_vec, compiler_error_list_t* error_list);

#endif // !COMPILER_LEXER
