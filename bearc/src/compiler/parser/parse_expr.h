#ifndef COMPILER_PARSER_EXPR_H
#define COMPILER_PARSER_EXPR_H
#include "compiler/ast/expr.h"
#include "compiler/parser/parser.h"
#include "utils/vector.h"

/// parse an expr
ast_expr_t parse_expr(parser_t parser);

/// for adding contents of the vector to the arena, and freeing the vector
/// - vector must contain type token_t*
token_ptr_slice_t parser_freeze_token_handle_slice(parser_t* p, vector_t* vec);

#endif
