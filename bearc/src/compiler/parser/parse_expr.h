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
#include "utils/spill_arr.h"
#include <stdint.h>

/// for adding contents of the vector to the arena, and freeing the vector
/// - vector must contain type ast_expr_t
ast_slice_of_exprs_t parser_freeze_expr_spill_arr(parser_t* p, spill_arr_ptr_t* sarr);

/// allocs an ast_expr_t using the parser's internal arena
ast_expr_t* parser_alloc_expr(parser_t* p);

/// parse an expr
ast_expr_t* parse_expr(parser_t* p);

/// lhs is NULLable
ast_expr_t* parse_expr_prec(parser_t* p, ast_expr_t* lhs, uint8_t prec);

/// lhs is not NULLable
ast_expr_t* parse_expr_with_leading_id_expr(parser_t* p, ast_expr_t* lhs);

ast_expr_t* parse_expr_from_id_slice(parser_t* p, token_ptr_slice_t id_slice);

ast_expr_t* parse_primary_expr(parser_t* p);

ast_expr_t* parse_preunary_expr(parser_t* p);

ast_expr_t* parse_literal(parser_t* p);

ast_expr_t* parse_id(parser_t* p);

token_t* parse_var_name(parser_t* p);

ast_expr_t* parse_binary(parser_t* p, ast_expr_t* lhs, uint8_t max_prec);

ast_expr_t* parse_postunary(parser_t* p, ast_expr_t* lhs);

ast_expr_t* parser_sync_expr(parser_t* p);

ast_expr_t* parse_grouping(parser_t* p);

ast_expr_t* parse_fn_call(parser_t* p, ast_expr_t* lhs);

ast_expr_t* parse_subscript(parser_t* p, ast_expr_t* lhs);

ast_expr_t* parse_expr_type(parser_t* p);

ast_expr_t* parse_expr_struct_init(parser_t* p, ast_expr_t* id_lhs);

ast_expr_t* parse_expr_borrow(parser_t* p);

ast_expr_t* parse_expr_variant_decomp(parser_t* p);

ast_expr_t* parse_expr_variant_decomp_with_leading_id(parser_t* p, token_ptr_slice_t id);

ast_expr_t* parse_expr_switch_pattern(parser_t* p);

ast_expr_t* parse_expr_allowing_block_exprs_with_yields(parser_t* p);

ast_expr_t* parse_expr_switch_branch(parser_t* p);

ast_expr_t* parse_expr_switch(parser_t* p);

ast_expr_t* parse_expr_closure(parser_t* p);

ast_expr_t* parse_expr_list_literal(parser_t* p);

#endif
