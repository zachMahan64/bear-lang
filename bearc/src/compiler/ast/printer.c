#include "compiler/ast/printer.h"
#include "compiler/token.h"
#include "utils/string.h"
#include <stddef.h>
#include <stdio.h>

static bool initialized;
static string_t indent_str;
static const char* indent = "  ";
#define PRINTER_INDENT_LEN 2

void printer_init(void) {
    static bool initialized;
    if (!initialized) {
        indent_str = string_create_and_reserve(36);
        // TODO
    }
}

void printer_do_indent(void) { string_push_cstr(&indent_str, indent); }

const char* printer_indent(void) { return string_data(&indent_str); }

void printer_deindent(void) { string_shrink_by(&indent_str, PRINTER_INDENT_LEN); }

void print_out_ast(ast_stmt_t* stmt) {
    // TODO
}

void print_expr(ast_expr_t* expression) {
    if (!initialized) {
        printer_init();
    }
    printer_do_indent();
    ast_expr_t expr = *expression;
    switch (expr.type) {
    case (AST_EXPR_ID): {
        token_ptr_slice_t ids = expression->expr.id.slice;
        for (size_t i = 0; i < ids.len; i++) {
            printf("%s%.*s\n", printer_indent(), (int)ids.start[i]->len, ids.start[i]->start);
        }
        break;
    }
    case AST_EXPR_LITERAL: {
        printf("%s%.*s\n", printer_indent(), (int)expr.expr.literal.tkn->len,
               expr.expr.literal.tkn->start);
    }
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
    printer_deindent();
}
