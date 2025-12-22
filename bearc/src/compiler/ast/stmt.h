//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef AST_STATEMENTS_H
#define AST_STATEMENTS_H
#include "compiler/ast/expr.h"
#include "compiler/ast/type.h"
#include "compiler/token.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    // blocks
    AST_STMT_BLOCK, // {...} sequence of statements
    AST_STMT_FILE,  // top-level flat seq of statements w/ metadata

    // var decls
    AST_STMT_VAR_DECL,      // type foo;
    AST_STMT_VAR_INIT_DECL, // type foo = something;

    // module stuff
    AST_STMT_MODULE,

    // import
    AST_STMT_IMPORT,

    // statement expr
    AST_STMT_EXPR,

    // statement empty
    AST_STMT_EMPTY,

    // function declarations
    AST_STMT_FN_DECL, // fn + params + (body for definitions / null for declarations)
                      // + return type

    // control flow
    AST_STMT_IF,     // KW_IF + condition + statement
    AST_STMT_ELSE,   // KW_ELSE (optional branch)
    AST_STMT_WHILE,  // KW_WHILE + condition + body
    AST_STMT_FOR,    // KW_FOR + init stmt + cond stmt + step expr + body stmt
    AST_STMT_FOR_IN, // KW_FOR + iterable + iterator + body stmt
    AST_STMT_RETURN, // KW_RETURN + expr

    // visibility
    AST_STMT_VISIBILITY_MODIFIER,

    // structs
    AST_STMT_STRUCT_DEF, // TOK_STRUCT + fields

    // marks
    AST_MARK_PREAMBLE,
    AST_MARK_DECL, // mark Name { functions }
    AST_STMT_INVALID,
} ast_stmt_type_e;

// forward decls ~~~~~~~~~~~~~~~~~~
typedef struct ast_stmt ast_stmt_t;
typedef struct ast_stmt_else ast_stmt_else_t;
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

/**
 * slice of statements
 */
typedef struct {
    ast_stmt_t** start;
    size_t len;
} ast_slice_of_stmts_t;

// stmt types ~~~~~~~~~~~~~~~~~~~~
/**
 * represents block statements
 * {...series of statements...}
 */
typedef struct {
    token_t* left_delim;
    ast_slice_of_stmts_t stmts;
    token_t* right_delim;
} ast_stmt_block_t;

/**
 * represents a top level sequence of statements in a file
 * this is the highest-order statement construct
 */
typedef struct {
    const char* file_name;
    ast_slice_of_stmts_t stmts;
} ast_stmt_file_t;

/**
 * represents a module decl or insertion in a module
 * mod my_mod {...} // module will enclosed
 */
typedef struct {
    token_t* id;
    ast_slice_of_stmts_t decls;
} ast_stmt_module_t;

/**
 * imports a file given my a path as my.path.to.file
 */
typedef struct {
    token_t* import;
    ast_expr_t* file_id;
} ast_stmt_import_t;

/// a statement expr, like `foo();`
typedef struct {
    /// sole-expr of this statement
    ast_expr_t* expr;
    /// ;
    token_t* terminator;
} ast_stmt_expr_t;

typedef struct {
    /// fn, mt, or dt
    token_t* kw;
    token_ptr_slice_t name;
    token_t* left_paren;
    ast_slice_of_params_t params;
    token_t* right_paren;
    /// NULLable if no return type
    token_t* rarrow;
    /// NULLable if no return type
    ast_type_t* return_type;
    ast_stmt_t* block;
} ast_stmt_fn_decl_t;

typedef struct {
    ast_type_t* type;
    token_t* name;
    token_t* assign_op;
    ast_expr_t* rhs;
    token_t* terminator;
} ast_stmt_var_decl_init_t;

typedef struct {
    ast_type_t* type;
    token_t* name;
    token_t* terminator;
} ast_stmt_var_decl_t;

typedef struct {
    ast_expr_t* condition;
    ast_stmt_t* stmt;
    /// NULLable if there's no else
    ast_stmt_t* else_stmt;
    bool has_else;
} ast_stmt_if_t;

typedef struct ast_stmt_else {
    ast_stmt_t* stmt;
} ast_stmt_else_t;

typedef struct {
    token_t* while_tkn;
    ast_stmt_t* stmt;
} ast_stmt_while_t;

typedef struct {
    token_t* for_tkn;
    ast_stmt_t* init;
    ast_stmt_t* condition;
    ast_expr_t* step;
    ast_stmt_t* body;
} ast_stmt_for_t;

typedef struct {
    token_t* for_tkn;
    ast_expr_id_t* type;
    ast_expr_t* iterable;
    token_t* in;
    ast_expr_t* iterator;
    ast_stmt_t* body;
} ast_stmt_for_in_t;

typedef struct {
    token_t* return_tkn;
    // optional
    ast_expr_t* expr;
    token_t* terminator;
} ast_stmt_return_t;

typedef struct {
    token_t* struct_tkn;
    ast_expr_id_t* name;
    ast_slice_of_stmts_t fields;
} ast_stmt_struct_decl_t;

typedef struct {
    token_t* mark_tkn;
    ast_expr_id_t* name;
    ast_stmt_block_t* funcs;
} ast_stmt_mark_decl_t;

typedef struct {
    token_t* hash_tkn;
    token_t* left_bracket;
    ast_slice_of_exprs_t marks;
    token_t* right_bracket;
} ast_stmt_mark_preamble_t;

typedef struct {
    token_t* terminator;
} ast_stmt_empty_t;

typedef struct {
    token_t* modifier;
    ast_stmt_t* stmt;
} ast_stmt_vis_modifier_t;

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

/// union of all stmt types
typedef union {
    ast_stmt_block_t block;
    ast_stmt_file_t file;
    ast_stmt_module_t module;
    ast_stmt_import_t import;
    ast_stmt_expr_t stmt_expr;
    ast_stmt_fn_decl_t fn_decl;
    ast_stmt_var_decl_init_t var_init_decl;
    ast_stmt_var_decl_t var_decl;
    ast_stmt_if_t if_stmt;
    ast_stmt_else_t else_stmt;
    ast_stmt_while_t while_stmt;
    ast_stmt_for_t for_stmt;
    ast_stmt_for_in_t for_in_stmt;
    ast_stmt_return_t return_stmt;
    ast_stmt_struct_decl_t struct_decl;
    ast_stmt_mark_decl_t mark_decl;
    ast_stmt_mark_preamble_t mark_preamble;
    ast_stmt_empty_t empty;
    ast_stmt_vis_modifier_t vis_modifier;
} ast_stmt_u;

typedef struct ast_stmt {
    ast_stmt_u stmt;
    ast_stmt_type_e type;
    token_t* first;
    token_t* last;
} ast_stmt_t;

#ifdef __cplusplus
} // extern "C"
#endif
#endif // !AST_STATEMENTS_H
