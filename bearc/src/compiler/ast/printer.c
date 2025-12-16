#include "compiler/ast/printer.h"
#include "compiler/token.h"
#include "utils/string.h"
#include <stddef.h>
#include <stdio.h>

static string_t indent_str;

void priner_int(void) {
    static bool initialized;
    if (!initialized) {
        indent_str = string_create_and_reserve(36);
        // TODO
    }
}

void print_out_ast(ast_stmt_t* stmt) {
    // TODO
}

void print_expr(ast_expr_t* expression) {
    ast_expr_t expr = *expression;
    switch (expr.type) {
    case (AST_EXPR_ID): {
        token_ptr_slice_t ids = expression->expr.id.slice;
        for (size_t i = 0; i < ids.len; i++) {
            printf("%.*s\n", (int)ids.start[i]->len, ids.start[i]->start);
        }
        break;
    }
    case AST_EXPR_LITERAL:
    case AST_EXPR_BINARY:
    case AST_EXPR_ASSIGN_EQ:
    case AST_EXPR_ASSIGN_MOVE:
    case AST_EXPR_GROUPING:
    case AST_EXPR_PRE_UNARY:
    case AST_EXPR_POST_UNARY:
    case AST_EXPR_FN_CALL:
    case AST_INVALID:
        break;
    }
}
