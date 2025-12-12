// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_LEXER
#define COMPILER_LEXER

#include "utils/file_io.h"
#include "utils/vector.h"

#define LEXER_ESTIMATED_CHARS_PER_TOKEN 6

/**
 * create a vector_t storing token_t from a specified src_buffer_t
 */
vector_t lexer_tokenize_src_buffer(const src_buffer_t* buf);

#endif // !COMPILER_LEXER
