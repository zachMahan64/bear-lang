#ifndef COMPILER_LEXER
#define COMPILER_LEXER

#include "containers/vector.h"

vector_t lexer_tokenize_src_buffer(const char* buffer);

#endif // !COMPILER_LEXER
