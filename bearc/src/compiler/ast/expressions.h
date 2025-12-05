#ifndef AST_EXPRESSIONS_H
#define AST_EXPRESSIONS_H
#include "compiler/token.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    AST_EXPR_MODULE_NAME, // for module blocks

    // expr
    AST_EXPR_PRIMARY,
    AST_EXPR_GROUPING,
    AST_EXPR_BINARY,      // +, -, *, /, %, bitwise, comparison, boolean
    AST_EXPR_UNARY,       // unary -, !, ~, ++, --
    AST_EXPR_ASSIGN_EQ,   // = or
    AST_EXPR_ASSIGN_MOVE, // <- assignment
    AST_EXPR_FN_CALL,     // func(args...)

    // atoms
    AST_LITERAL,  // INT_LIT, FLOAT_LIT, CHAR_LIT, STRING_LIT
    AST_VARIABLE, // SCOPE ... + SYMBOL
    AST_SELF,     // TOK_SELF keyword

} ast_expr_type_e;

typedef struct {
    token_t* tkn;
} ast_expr_module_name;

typedef struct {
    token_t* tkn;
} ast_expr_primary;

typedef union {

} ast_expr_u;

typedef struct {
    ast_expr_type_e type;
    ast_expr_u expr;
} ast_expr_t;

#ifdef __cplusplus
} // extern "C"
#endif
#endif // !AST_STATEMENTS_H
