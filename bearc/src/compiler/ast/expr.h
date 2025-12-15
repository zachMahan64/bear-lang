//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef AST_EXPRESSIONS_H
#define AST_EXPRESSIONS_H
#include "compiler/token.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    // atoms/primary exprs
    AST_EXPR_ID,
    AST_EXPR_LITERAL,
    // binary
    AST_EXPR_BIN_ARITH, // binary arithmetic expression: +, -, *, /, %, bitwise, comparison, boolean
    // assign
    AST_EXPR_ASSIGN_EQ,   // binary copy assignment: expr = expr
    AST_EXPR_ASSIGN_MOVE, // binary move assignment: expr <- expr
    // grouping
    AST_EXPR_GROUPING, // (<some_expr>)
    // unary
    AST_EXPR_PRE_UNARY,  // unary expr: -, +, !, ~, ++, --
    AST_EXPR_POST_UNARY, // unary expr: ++, --
    // func
    AST_EXPR_FN_CALL, // func(args...), can also be used for methods

} ast_expr_type_e;

// main generic expr type
typedef struct ast_expr ast_expr_t;

/// slice of ast_expr_t
typedef struct {
    ast_expr_t* start;
    size_t len;
} ast_slice_of_exprs_t;

// expr types ~~~~~~~~~~~~

typedef struct {
    token_ptr_slice_t slice;
} ast_expr_id_t;

typedef struct ast_expr_literal {
    token_t* tkn;
} ast_expr_literal_t;

// resolve through operator token type
typedef struct {
    ast_expr_t* lhs;
    token_t* op;
    ast_expr_t* rhs;
} ast_expr_binary_t;

typedef struct {
    ast_expr_t* lval;
    token_t* assign_op;
    ast_expr_t* rhs;
} ast_expr_assign_t;

typedef struct {
    token_t* left_paren;
    ast_expr_t* expr;
    token_t* right_paren;
} ast_expr_grouping_t;

// pre/postfix must be determined by the ast_expr_type_e inside the wrapping ast_expr_t
typedef struct {
    ast_expr_t* expr;
    token_t* op;
} ast_expr_unary_t;

typedef struct {
    ast_expr_t* expr; // should resolve to a func/func ptr
    token_t* left_paren;
    ast_slice_of_exprs_t args_vec; // of type ast_expr_t
    token_t* right_paren;
} ast_expr_fn_call_t;

// ^^^^^^^^^^^^^^^^^^^^^^^^

typedef union {
    ast_expr_id_t id;
    ast_expr_literal_t literal;
    ast_expr_binary_t binary;
    ast_expr_assign_t assign;
    ast_expr_grouping_t grouping;
    ast_expr_unary_t unary;
    ast_expr_fn_call_t fn_call;
} ast_expr_u;

/// underlying expr is 0-offset alligned so this struct can be safely downcasted
typedef struct ast_expr {
    ast_expr_u expr;
    ast_expr_type_e type;
    token_t* first;
    token_t* last;
} ast_expr_t;

#ifdef __cplusplus
} // extern "C"
#endif
#endif // !AST_STATEMENTS_H
