#include "compiler/ast/printer.h"
#include "compiler/token.h"
#include "utils/ansi_codes.h"
#include "utils/string.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

static string_t indent_str;
static const char* indent = "  ";
#define PRINTER_INDENT_LEN 2

void printer_try_init(void) {
    static bool initialized;
    if (!initialized) {
        indent_str = string_create_and_reserve(36);
        initialized = true;
    }
}

void printer_do_indent(void) { string_push_cstr(&indent_str, indent); }

const char* printer_indent(void) { return string_data(&indent_str); }

void printer_deindent(void) { string_shrink_by(&indent_str, PRINTER_INDENT_LEN); }

void print_indent(void) { printf("%s", string_data(&indent_str)); }

void print_out_ast(ast_stmt_t* stmt) {
    // TODO
}

static void print_tkn(token_t* tkn) { printf("%.*s", (int)tkn->len, tkn->start); }

void print_expr(ast_expr_t* expression) {
    printer_try_init();
    printer_do_indent();
    print_indent();
    ast_expr_t expr = *expression;
    switch (expr.type) {
    case (AST_EXPR_ID): {
        token_ptr_slice_t ids = expression->expr.id.slice;
        printf("identifer: " ANSI_BOLD_GREEN "`" ANSI_RESET);
        for (size_t i = 0; i < ids.len; i++) {
            int len = (int)ids.start[i]->len;
            const char* start = ids.start[i]->start;
            printf("%.*s", len, start);
            if (ids.len != 1 && i != ids.len - 1) {
                printf(ANSI_BOLD_GREEN "%s" ANSI_RESET, get_token_to_string_map()[TOK_SCOPE_RES]);
            }
        }
        printf(ANSI_BOLD_GREEN "`" ANSI_RESET);
        break;
    }
    case AST_EXPR_LITERAL: {
        token_t* tkn = expr.expr.literal.tkn;
        const char* lit_type_str = get_token_to_string_map()[tkn->type];
        printf("literal (%s): %.*s", lit_type_str, (int)tkn->len, tkn->start);
        break;
    }
    case AST_EXPR_BINARY: {
        puts("binary op: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        ast_expr_t* lhs = expr.expr.binary.lhs;
        token_t* op = expr.expr.binary.op;
        ast_expr_t* rhs = expr.expr.binary.rhs;
        print_expr(lhs);
        print_indent(), printf("  op: "), print_tkn(op), puts(",");
        print_expr(rhs);
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    }
    case AST_EXPR_ASSIGN_EQ:
    case AST_EXPR_ASSIGN_MOVE:
    case AST_EXPR_GROUPING: {
        puts("grouping: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_expr(expr.expr.grouping.expr);
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    }
    case AST_EXPR_PRE_UNARY: {
        token_t* op = expr.expr.unary.op;
        puts("pre-unary: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_indent(), printf("  op: "), print_tkn(op), puts(",");
        print_expr(expr.expr.unary.expr);
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    }
    case AST_EXPR_POST_UNARY:
    case AST_EXPR_FN_CALL:
    case AST_INVALID:
        break;
    }
    puts(",");
    printer_deindent();
}
