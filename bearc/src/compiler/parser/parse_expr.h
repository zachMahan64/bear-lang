//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_PARSER_EXPR_H
#define COMPILER_PARSER_EXPR_H
#include "compiler/ast/expr.h"
#include "compiler/parser/parser.h"
#include "utils/vector.h"

/// for adding contents of the vector to the arena, and freeing the vector
/// - vector must contain type token_t*
token_ptr_slice_t parser_freeze_token_ptr_slice(parser_t* p, vector_t* vec);

/// for adding contents of the vector to the arena, and freeing the vector
/// - vector must contain type ast_expr_t
ast_slice_of_exprs_t parser_freeze_expr_vec(parser_t* p, vector_t* vec);

/// allocs an ast_expr_t using the parser's internal arena
ast_expr_t* parser_alloc_expr(parser_t* p);

/// parse an expr
ast_expr_t* parse_expr(parser_t* p);

ast_expr_t* parse_preunary_expr(parser_t* p);

ast_expr_t* parse_literal(parser_t* p);

ast_expr_t* parse_id(parser_t* p);

ast_expr_t* parse_binary(parser_t* p, ast_expr_t* lhs);

ast_expr_t* parser_sync(parser_t* p);

#endif
