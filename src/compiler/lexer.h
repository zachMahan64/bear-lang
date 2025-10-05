#ifndef COMPILER_LEXER
#define COMPILER_LEXER

#include "containers/vector.h"
#include "file_io.h"

#define LEXER_ESTIMATED_CHARS_PER_TOKEN 6

// create a vector storing token_t from a specified src_buffer_t
vector_t lexer_naively_by_whitespace_tokenize_src_buffer(const src_buffer_t* src_buffer);
vector_t lexer_tokenize_src_buffer(const src_buffer_t* buf);

#endif // !COMPILER_LEXER
