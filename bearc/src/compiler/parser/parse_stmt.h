//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_PARSER_STMT
#define COMPILER_PARSER_STMT

#include "compiler/ast/stmt.h"
#include "compiler/parser/parser.h"

/// builds up an ast in the form of a file stmt which contains a file_name and a vector of
/// ast_stmt_t's
ast_stmt_t* parse_file(parser_t* parser, const char* file_name);

/// for adding contents of the vector to the arena, and freeing the vector
/// - vector must contain type ast_stmt_t
ast_slice_of_stmts_t parser_freeze_stmt_vec(parser_t* p, vector_t* vec);

ast_slice_of_stmts_t parse_slice_of_stmts(parser_t* p, token_type_e until);

ast_stmt_t* parser_alloc_stmt(parser_t* p);

ast_stmt_t* parse_stmt(parser_t* p);

ast_stmt_t* parse_stmt_expr(parser_t* p, ast_expr_t* expr);

ast_stmt_t* parse_stmt_block(parser_t* p);

ast_stmt_t* parse_var_decl(parser_t* p, ast_expr_t* id_expr, bool leading_mut);

#endif
