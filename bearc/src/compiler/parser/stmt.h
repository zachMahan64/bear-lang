#ifndef COMPILER_PARSER_STMT
#define COMPILER_PARSER_STMT

#include "compiler/ast/stmt.h"
#include "compiler/parser/parser.h"

/// builds up an ast in the form of a file stmt which contains a file_name and a vector of
/// ast_stmt_t's
ast_stmt_file_t parser_file(parser_t* parser, const char* file_name, vector_t token_vec);

#endif
