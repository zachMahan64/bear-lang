//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/ast/expr.h"
#include "compiler/token.h"
#include <stdbool.h>
#include <stddef.h>

#ifndef COMPILER_AST_TYPE
#define COMPILER_AST_TYPE

typedef enum {
    AST_TYPE_BASE,
    AST_TYPE_REF_PTR,
    AST_TYPE_ARR,
    AST_TYPE_SLICE,
    AST_TYPE_GENERIC,
    AST_TYPE_INVALID,
} ast_type_e;

typedef struct {
    token_ptr_slice_t id;
    bool mut;
} ast_type_base_t;

// shared by tags AST_TYPE_REF and AST_TYPE_PTR
typedef struct {
    ast_type_t* inner;
    token_t* modifier; // & or *
    bool mut;
} ast_type_ref_t;

typedef struct {
    ast_type_t* inner;
    ast_expr_t* size_expr;
} ast_type_arr_t;

typedef struct {
    ast_type_t* inner;
    bool mut;
} ast_type_slice_t;

typedef struct {
    ast_type_t* inner;
    ast_slice_of_generic_args_t generic_args;
} ast_type_generic_t;

typedef union {
    ast_type_base_t base;
    ast_type_ref_t ref;
    ast_type_arr_t arr;
    ast_type_generic_t generic;
    ast_type_slice_t slice;
} ast_type_u;

typedef struct ast_type {
    ast_type_u type;
    ast_type_e tag;
    ast_type_t* canonical_base;
    token_t* first;
    token_t* last;
} ast_type_t;

typedef struct {
    ast_type_t* type;
    token_t* name;
    bool valid;
    token_t* first;
    token_t* last;
} ast_param_t;

typedef struct {
    ast_param_t** start;
    size_t len;
} ast_slice_of_params_t;

typedef struct {
    token_t* id;
    ast_slice_of_exprs_t mark_ids;
    bool valid;
} ast_type_with_marks_t;

#endif // !COMPILER_AST_TYPE
