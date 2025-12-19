#include "compiler/ast/printer.h"
#include "compiler/ast/expr.h"
#include "compiler/ast/stmt.h"
#include "compiler/token.h"
#include "utils/ansi_codes.h"
#include "utils/string.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

static string_t indent_str;
static const char* indent = "|   ";
#define PRINTER_INDENT_LEN 4

static void printer_try_init(void) {
    static bool initialized;
    if (!initialized) {
        indent_str = string_create_and_reserve(36);
        initialized = true;
    }
}

static void printer_do_indent(void) { string_push_cstr(&indent_str, indent); }

static void printer_deindent(void) { string_shrink_by(&indent_str, PRINTER_INDENT_LEN); }

static void print_indent(void) { printf("%s", string_data(&indent_str)); }

static void print_tkn(token_t* tkn) {
    if (!tkn) {
        printf(ANSI_BOLD_RED "missing tkn" ANSI_RESET);
        return;
    }
    printf("%.*s", (int)tkn->len, tkn->start);
}

static void print_op(token_t* op) {
    printer_do_indent(), print_indent(), printf(ANSI_BOLD_GREEN "`" ANSI_RESET ANSI_BOLD_MAGENTA),
        print_tkn(op), puts(ANSI_BOLD_GREEN "`" ANSI_RESET ","), printer_deindent();
}

static void print_var_name(token_t* name) {
    printer_do_indent(), print_indent(),
        printf("name: " ANSI_BOLD_GREEN "`" ANSI_RESET ANSI_BOLD_CYAN), print_tkn(name),
        puts(ANSI_BOLD_GREEN "`" ANSI_RESET ","), printer_deindent();
}

static void print_comma(void) {
    printer_do_indent(), print_indent(),
        printf(ANSI_BOLD_GREEN "`" ANSI_RESET ANSI_BOLD_YELLOW "," ANSI_BOLD_GREEN "`" ANSI_RESET
                               ",\n"),
        printer_deindent();
}

static void print_opening_delim(token_t* delim) {
    printer_do_indent(), print_indent(), printf(ANSI_BOLD_GREEN "`" ANSI_RESET ANSI_BOLD_YELLOW),
        print_tkn(delim), puts(ANSI_BOLD_GREEN "`" ANSI_RESET ",");
}

static void print_closing_delim(token_t* delim) {
    print_indent(), printf(ANSI_BOLD_GREEN "`" ANSI_RESET ANSI_BOLD_YELLOW), print_tkn(delim),
        puts(ANSI_BOLD_GREEN "`" ANSI_RESET ","), printer_deindent();
}

static void print_opening_delim_from_type(token_type_e delim) {
    printer_do_indent(), print_indent(),
        printf(ANSI_BOLD_GREEN "`" ANSI_RESET ANSI_BOLD_YELLOW "%s",
               get_token_to_string_map()[delim]),
        puts(ANSI_BOLD_GREEN "`" ANSI_RESET ",");
}

static void print_closing_delim_from_type(token_type_e delim) {
    print_indent(),
        printf(ANSI_BOLD_GREEN "`" ANSI_RESET ANSI_BOLD_YELLOW "%s",
               get_token_to_string_map()[delim]),
        puts(ANSI_BOLD_GREEN "`" ANSI_RESET ","), printer_deindent();
}

static void print_terminator(token_t* term) {
    if (!term) {
        print_indent(), puts(ANSI_BOLD_RED "missing terminator" ANSI_RESET);
        return;
    }
    print_indent(), printf(ANSI_BOLD_GREEN "`" ANSI_RESET ANSI_BOLD_YELLOW), print_tkn(term),
        puts(ANSI_BOLD_GREEN "`" ANSI_RESET ",");
}

static void print_mut(void) {
    print_indent(),
        printf(ANSI_BOLD_GREEN "`" ANSI_BOLD_MAGENTA "mut" ANSI_BOLD_GREEN "`" ANSI_RESET ",\n");
}

static void print_type(ast_type_t* type) {
    printer_do_indent();
    print_indent();

    switch (type->tag) {
    case AST_TYPE_BASE: {
        token_ptr_slice_t ids = type->type.base.id;
        puts("base type: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        printer_do_indent();
        print_indent();
        printf(ANSI_BOLD_GREEN "`" ANSI_RESET);
        for (size_t i = 0; i < ids.len; i++) {
            int len = (int)ids.start[i]->len;
            const char* start = ids.start[i]->start;
            printf(ANSI_BOLD_YELLOW "%.*s" ANSI_RESET, len, start);
            if (ids.len != 1 && i != ids.len - 1) {
                printf(ANSI_BOLD_GREEN "%s" ANSI_RESET, get_token_to_string_map()[TOK_SCOPE_RES]);
            }
        }
        printf(ANSI_BOLD_GREEN "`" ANSI_RESET);
        printf(",\n");
        if (type->type.base.mut) {
            print_mut();
        }
        printer_deindent();
        print_indent(), printf(ANSI_BOLD_GREEN "}\n" ANSI_RESET);
        break;
    }
    case AST_TYPE_REF_PTR: {
        puts("ref/ptr type: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_type(type->type.ref.inner);
        print_op(type->type.ref.modifier);
        if (type->type.ref.mut) {
            printer_do_indent();
            print_mut();
            printer_deindent();
        }
        print_indent(), printf(ANSI_BOLD_GREEN "}\n" ANSI_RESET);
        break;
    }
    case AST_TYPE_INVALID:
        break;
    }
    printer_deindent();
}

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
            printf(ANSI_BOLD_CYAN "%.*s" ANSI_RESET, len, start);
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
        printf("literal (%s): " ANSI_BOLD_GREEN "`" ANSI_RESET ANSI_BOLD_BLUE "%.*s" ANSI_BOLD_GREEN
               "`" ANSI_RESET,
               lit_type_str, (int)tkn->len, tkn->start);
        break;
    }
    case AST_EXPR_BINARY: {
        puts("binary op: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        ast_expr_t* lhs = expr.expr.binary.lhs;
        token_t* op = expr.expr.binary.op;
        ast_expr_t* rhs = expr.expr.binary.rhs;
        print_expr(lhs);
        print_op(op);
        print_expr(rhs);
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    }
    case AST_EXPR_GROUPING: {
        puts("grouping: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_expr(expr.expr.grouping.expr);
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    }
    case AST_EXPR_PRE_UNARY: {
        token_t* op = expr.expr.unary.op;
        puts("pre-unary: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_op(op);
        print_expr(expr.expr.unary.expr);
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    }
    case AST_EXPR_POST_UNARY: {
        token_t* op = expr.expr.unary.op;
        puts("post-unary: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_expr(expr.expr.unary.expr);
        print_op(op);
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    }
    case AST_EXPR_FN_CALL:
        puts("function call: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_expr(expr.expr.fn_call.left_expr);
        print_opening_delim(expr.expr.fn_call.left_paren);

        ast_slice_of_exprs_t args = expr.expr.fn_call.args;
        for (size_t i = 0; i < args.len; i++) {
            print_expr(args.start[i]);
            if (i != args.len - 1) {
                print_comma();
            }
        }

        print_closing_delim(expr.expr.fn_call.right_paren);
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    case AST_EXPR_INVALID:
        printf(ANSI_BOLD_RED "invalid expression" ANSI_RESET);
        break;
    case AST_EXPR_SUBSCRIPT:
        puts("subscript: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_expr(expr.expr.subscript.lhs);
        print_opening_delim_from_type(TOK_LBRACK);
        print_expr(expr.expr.subscript.subexpr);
        print_closing_delim_from_type(TOK_RBRACK);
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    }
    puts(",");
    printer_deindent();
}

void print_stmt(ast_stmt_t* stmt) {
    printer_try_init();
    print_indent();
    switch (stmt->type) {
    case AST_STMT_BLOCK:
        printf("block: " ANSI_BOLD_GREEN "{\n" ANSI_RESET);
        print_opening_delim_from_type(TOK_LBRACE);
        for (size_t i = 0; i < stmt->stmt.block.stmts.len; i++) {
            print_stmt(stmt->stmt.block.stmts.start[i]);
        }
        print_closing_delim_from_type(TOK_RBRACE);
        print_indent(), printf(ANSI_BOLD_GREEN "}\n" ANSI_RESET);
        break;
    case AST_STMT_MODULE_BLOCK:
    case AST_STMT_MODULE_FLAT:
    case AST_STMT_FILE:
        printf("file '%s': " ANSI_BOLD_GREEN "{\n" ANSI_RESET, stmt->stmt.file.file_name);
        for (size_t i = 0; i < stmt->stmt.file.stmts.len; i++) {
            print_stmt(stmt->stmt.file.stmts.start[i]);
        }
        print_indent(), printf(ANSI_BOLD_GREEN "}\n" ANSI_RESET);
        break;
    case AST_STMT_IMPORT:
    case AST_STMT_EXPR:
        puts("expression-statement: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_expr(stmt->stmt.stmt_expr.expr);
        print_terminator(stmt->stmt.stmt_expr.terminator);
        print_indent(), printf(ANSI_BOLD_GREEN "}\n" ANSI_RESET);
        break;
    case AST_STMT_FN_DECL:
        puts("function declaration: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        ast_stmt_fn_decl_t fn = stmt->stmt.fn_decl;
        print_op(fn.kw);
        printer_do_indent();
        print_indent();
        printf(ANSI_BOLD_GREEN "`" ANSI_RESET);
        token_ptr_slice_t ids = fn.name;
        for (size_t i = 0; i < ids.len; i++) {
            int len = (int)ids.start[i]->len;
            const char* start = ids.start[i]->start;
            printf(ANSI_BOLD_CYAN "%.*s" ANSI_RESET, len, start);
            if (ids.len != 1 && i != ids.len - 1) {
                printf(ANSI_BOLD_GREEN "%s" ANSI_RESET, get_token_to_string_map()[TOK_SCOPE_RES]);
            }
        }
        printer_deindent();
        printf(ANSI_BOLD_GREEN "`" ANSI_RESET);
        printf(",\n");
        print_opening_delim(fn.left_paren);
        print_closing_delim(fn.right_paren);
        if (fn.rarrow) {
            if (fn.rarrow->type == TOK_RARROW) {
                print_op(fn.rarrow);
                print_type(fn.return_type);
            }
        }
        printer_do_indent();
        print_stmt(fn.block);
        printer_deindent();
        print_indent(), printf(ANSI_BOLD_GREEN "}\n" ANSI_RESET);
        break;
    case AST_STMT_VAR_INIT_DECL:
        puts("variable initialization: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_type(stmt->stmt.var_init_decl.type);
        print_var_name(stmt->stmt.var_init_decl.name);
        print_op(stmt->stmt.var_init_decl.assign_op);
        print_expr(stmt->stmt.var_init_decl.rhs);
        print_terminator(stmt->stmt.var_init_decl.terminator);
        print_indent(), printf(ANSI_BOLD_GREEN "}\n" ANSI_RESET);
        break;
    case AST_STMT_VAR_DECL:
        puts("variable declaration: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_type(stmt->stmt.var_decl.type);
        print_var_name(stmt->stmt.var_decl.name);
        print_terminator(stmt->stmt.var_decl.terminator);
        print_indent(), printf(ANSI_BOLD_GREEN "}\n" ANSI_RESET);
        break;
    case AST_STMT_IF:
    case AST_STMT_ELSE:
    case AST_STMT_WHILE:
    case AST_STMT_FOR:
    case AST_STMT_FOR_IN:
    case AST_STMT_RETURN:
        puts("return statement: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_op(stmt->stmt.return_stmt.return_tkn);
        ast_expr_t* expr = stmt->stmt.return_stmt.expr;
        if (expr) {
            print_expr(expr);
        }
        print_terminator(stmt->stmt.return_stmt.terminator);
        print_indent(), printf(ANSI_BOLD_GREEN "}\n" ANSI_RESET);
        break;
    case AST_STMT_STRUCT_DEF:
    case AST_MARK_PREAMBLE:
    case AST_MARK_DECL:
    case AST_STMT_INVALID:
        puts(ANSI_BOLD_RED "invalid statement" ANSI_RESET);
        break;
    case AST_STMT_EMPTY:
        puts("empty statement: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_terminator(stmt->stmt.empty.terminator);
        print_indent(), printf(ANSI_BOLD_GREEN "}\n" ANSI_RESET);
        break;
    }
}
