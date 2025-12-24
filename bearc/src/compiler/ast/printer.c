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
// make sure to adjust these so they match or it'll look very ugly:
static const char* indent = "|   ";
#define PRINTER_INDENT_LEN 4
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

static void print_delineator_from_type(token_type_e delin) {
    printer_do_indent();
    print_indent(),
        printf(ANSI_BOLD_GREEN "`" ANSI_RESET ANSI_BOLD_YELLOW "%s",
               get_token_to_string_map()[delin]),
        puts(ANSI_BOLD_GREEN "`" ANSI_RESET ",");
    printer_deindent();
}

static void print_terminator(token_t* term) {
    if (!term) {
        print_indent(), puts(ANSI_BOLD_RED "missing terminator" ANSI_RESET);
        return;
    }
    print_indent(), printf(ANSI_BOLD_GREEN "`" ANSI_RESET ANSI_BOLD_YELLOW), print_tkn(term),
        puts(ANSI_BOLD_GREEN "`" ANSI_RESET ",");
}

static void print_op_from_type(token_type_e t) {
    print_indent(),
        printf(ANSI_BOLD_GREEN "`" ANSI_BOLD_MAGENTA "%s" ANSI_BOLD_GREEN "`" ANSI_RESET ",\n",
               get_token_to_string_map()[t]);
}

static void print_mut(void) { print_op_from_type(TOK_MUT); }

static void print_type(ast_type_t* type);

static void print_generic_type_arg(ast_generic_arg_t* arg) {
    if (arg->valid) {
        if (arg->tag == AST_GENERIC_ARG_TYPE) {
            print_type(arg->arg.type);
        } else if (arg->tag == AST_GENERIC_ARG_EXPR) {
            print_expr(arg->arg.expr);
        }
    } else {
        printer_do_indent();
        print_indent(), printf(ANSI_BOLD_RED "invalid generic arg" ANSI_RESET ",\n");
        printer_deindent();
    }
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
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
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
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    }
    case AST_TYPE_ARR:
        puts("arr type: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_opening_delim_from_type(TOK_LBRACK);
        print_expr(type->type.arr.size_expr);
        print_closing_delim_from_type(TOK_RBRACK);
        print_type(type->type.arr.inner);
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);

        break;
    case AST_TYPE_GENERIC:
        puts("generic type: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_type(type->type.generic.inner);
        print_delineator_from_type(TOK_GENERIC_SEP);
        print_opening_delim_from_type(TOK_LT);
        for (size_t i = 0; i < type->type.generic.generic_args.len; i++) {
            print_generic_type_arg(type->type.generic.generic_args.start[i]);
        }
        print_closing_delim_from_type(TOK_GT);
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    case AST_TYPE_INVALID:
        print_indent();
        printf(ANSI_BOLD_RED "invalid type" ANSI_RESET);
        break;
    case AST_TYPE_SLICE:
        puts("slice type: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_opening_delim_from_type(TOK_LBRACK);
        print_op_from_type(TOK_AMPER);
        if (type->type.slice.mut) {
            print_mut();
        }
        print_closing_delim_from_type(TOK_RBRACK);
        print_type(type->type.slice.inner);
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    }
    puts(",");
    printer_deindent();
}

static void print_param(ast_param_t* param) {
    print_indent();
    if (!param->valid) {
        printf(ANSI_BOLD_RED "invalid parameter,\n" ANSI_RESET);
        return;
    }
    puts("parameter: " ANSI_BOLD_GREEN "{" ANSI_RESET);
    print_type(param->type);
    print_var_name(param->name);
    print_indent(), printf(ANSI_BOLD_GREEN "},\n" ANSI_RESET);
}

static void print_id_slice(token_ptr_slice_t ids) {
    printer_do_indent();
    printf(ANSI_BOLD_GREEN "`" ANSI_RESET);
    for (size_t i = 0; i < ids.len; i++) {
        int len = (int)ids.start[i]->len;
        const char* start = ids.start[i]->start;
        printf(ANSI_BOLD_CYAN "%.*s" ANSI_RESET, len, start);
        if (ids.len != 1 && i != ids.len - 1) {
            printf(ANSI_BOLD_GREEN "%s" ANSI_RESET, get_token_to_string_map()[TOK_SCOPE_RES]);
        }
    }
    printf(ANSI_BOLD_GREEN "`" ANSI_RESET);
    printer_deindent();
}

static void print_generic_param_type(ast_param_generic_type_t* t) {
    print_indent();
    if (!t->valid) {
        printf(ANSI_BOLD_RED "invalid parameter,\n" ANSI_RESET);
        return;
    }
    puts("generic type parameter: " ANSI_BOLD_GREEN "{" ANSI_RESET);
    print_var_name(t->id);
    if (t->mark_ids.len != 0) {
        print_op_from_type(TOK_HAS);
        print_opening_delim_from_type(TOK_LPAREN);
        for (size_t i = 0; i < t->mark_ids.len; i++) {
            print_expr(t->mark_ids.start[i]);
        }
        print_closing_delim_from_type(TOK_RPAREN);
    }
    print_indent(), printf(ANSI_BOLD_GREEN "},\n" ANSI_RESET);
}

static void print_generic_params(ast_slice_of_generic_params_t params) {
    print_indent();
    puts("generic parameter list: " ANSI_BOLD_GREEN "{" ANSI_RESET);
    printer_do_indent();
    for (size_t i = 0; i < params.len; i++) {
        switch (params.start[i]->tag) {
        case AST_GENERIC_PARAM_TYPE:
            print_generic_param_type(params.start[i]->param.generic_type);
            break;
        case AST_GENERIC_PARAM_VAR:
            print_param(params.start[i]->param.generic_var);
            break;
        case AST_GENERIC_PARAM_INVALID:
            print_indent(), printf(ANSI_BOLD_RED "invalid generic param" ANSI_RESET ",\n");
            break;
        }
    }
    printer_deindent();
    print_indent(), printf(ANSI_BOLD_GREEN "},\n" ANSI_RESET);
}

void print_expr(ast_expr_t* expression) {
    printer_try_init();
    printer_do_indent();
    print_indent();
    ast_expr_t expr = *expression;
    switch (expr.type) {
    case (AST_EXPR_ID): {
        token_ptr_slice_t ids = expression->expr.id.slice;
        printf("identifier: ");
        print_id_slice(ids);
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
        if (expr.expr.fn_call.is_generic) {
            ast_slice_of_generic_args_t args = expr.expr.fn_call.generic_args;
            print_delineator_from_type(TOK_GENERIC_SEP);
            print_opening_delim_from_type(TOK_LT);
            for (size_t i = 0; i < args.len; i++) {
                print_generic_type_arg(args.start[i]);
            }
            print_closing_delim_from_type(TOK_GT);
        }
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
    case AST_EXPR_TYPE:
        puts("type expression: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_type(expr.expr.type_expr.type);
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
        break;
    case AST_STMT_MODULE:
        printf("module: " ANSI_BOLD_GREEN "{\n" ANSI_RESET);
        printer_do_indent();
        print_op_from_type(TOK_MODULE);
        printer_deindent();
        print_var_name(stmt->stmt.module.id);
        printer_do_indent();
        for (size_t i = 0; i < stmt->stmt.file.stmts.len; i++) {
            print_stmt(stmt->stmt.module.decls.start[i]);
        }
        printer_deindent();
        break;
    case AST_STMT_FILE:
        printf("file '%s': " ANSI_BOLD_GREEN "{\n" ANSI_RESET, stmt->stmt.file.file_name);
        for (size_t i = 0; i < stmt->stmt.file.stmts.len; i++) {
            print_stmt(stmt->stmt.file.stmts.start[i]);
        }
        break;
    case AST_STMT_IMPORT:
        printf("import statement: " ANSI_BOLD_GREEN "{\n" ANSI_RESET);
        printer_do_indent();
        print_op_from_type(TOK_IMPORT);
        print_closing_delim(stmt->stmt.import.file_path);
        puts(",");
        printer_deindent();
        break;
    case AST_STMT_EXPR:
        puts("expression-statement: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_expr(stmt->stmt.stmt_expr.expr);
        print_terminator(stmt->stmt.stmt_expr.terminator);
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
        printf(ANSI_BOLD_GREEN "`" ANSI_RESET);
        printf(",\n");
        if (fn.is_generic) {
            print_generic_params(fn.generic_params);
        }
        printer_deindent();
        print_opening_delim(fn.left_paren);
        for (size_t i = 0; i < fn.params.len; i++) {
            print_param(fn.params.start[i]);
        }
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
        break;
    case AST_STMT_VAR_INIT_DECL:
        puts("variable initialization: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_type(stmt->stmt.var_init_decl.type);
        print_var_name(stmt->stmt.var_init_decl.name);
        print_op(stmt->stmt.var_init_decl.assign_op);
        print_expr(stmt->stmt.var_init_decl.rhs);
        print_terminator(stmt->stmt.var_init_decl.terminator);
        break;
    case AST_STMT_VAR_DECL:
        puts("variable declaration: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_type(stmt->stmt.var_decl.type);
        print_var_name(stmt->stmt.var_decl.name);
        print_terminator(stmt->stmt.var_decl.terminator);
        break;
    case AST_STMT_IF:
        puts("if statement: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        printer_do_indent();
        print_op_from_type(TOK_IF);
        printer_deindent();
        print_expr(stmt->stmt.if_stmt.condition);
        printer_do_indent();
        print_stmt(stmt->stmt.if_stmt.body_stmt);
        printer_deindent();
        if (stmt->stmt.if_stmt.has_else) {
            printer_do_indent();
            print_stmt(stmt->stmt.if_stmt.else_stmt);
            printer_deindent();
        }
        break;
    case AST_STMT_ELSE:
        puts("else statement: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        printer_do_indent();
        print_op_from_type(TOK_ELSE);
        print_stmt(stmt->stmt.else_stmt.body_stmt);
        printer_deindent();
        break;
    case AST_STMT_WHILE:
        puts("while-loop statement: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        printer_do_indent();
        print_op_from_type(TOK_WHILE);
        printer_deindent();
        print_expr(stmt->stmt.while_stmt.condition);
        printer_do_indent();
        print_stmt(stmt->stmt.while_stmt.body_stmt);
        printer_deindent();
        break;
    case AST_STMT_FOR:
        puts("for-loop statement: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        printer_do_indent();
        print_op_from_type(TOK_FOR);
        print_stmt(stmt->stmt.for_stmt.init);
        printer_deindent();
        print_expr(stmt->stmt.for_stmt.condition);
        print_expr(stmt->stmt.for_stmt.step);
        printer_do_indent();
        print_stmt(stmt->stmt.for_stmt.body_stmt);
        printer_deindent();
        break;
    case AST_STMT_FOR_IN:
        puts("for-in-loop statement: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        printer_do_indent();
        print_op_from_type(TOK_FOR);
        print_param(stmt->stmt.for_in_stmt.each);
        printer_deindent();
        print_delineator_from_type(TOK_IN);
        print_expr(stmt->stmt.for_in_stmt.iterator);
        printer_do_indent();
        print_stmt(stmt->stmt.for_in_stmt.body_stmt);
        printer_deindent();
        break;
    case AST_STMT_RETURN:
        puts("return statement: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_op(stmt->stmt.return_stmt.return_tkn);
        ast_expr_t* expr = stmt->stmt.return_stmt.expr;
        if (expr) {
            print_expr(expr);
        }
        print_terminator(stmt->stmt.return_stmt.terminator);
        break;
    case AST_STMT_STRUCT_DEF:
    case AST_MARK_PREAMBLE:
    case AST_MARK_DECL:
        break;
    case AST_STMT_INVALID:
        puts(ANSI_BOLD_RED "invalid statement" ANSI_BOLD_GREEN " {" ANSI_RESET);
        break;
    case AST_STMT_EMPTY:
        puts("empty statement: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_terminator(stmt->stmt.empty.terminator);
        break;
    case AST_STMT_VISIBILITY_MODIFIER:
        puts("visibility-modified declaration: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_op(stmt->stmt.vis_modifier.modifier);
        printer_do_indent();
        print_stmt(stmt->stmt.vis_modifier.stmt);
        printer_deindent();
        break;
    case AST_STMT_BREAK:
        puts("break statement: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        printer_do_indent();
        print_op_from_type(TOK_BREAK);
        printer_deindent();
        print_delineator_from_type(TOK_SEMICOLON);
        break;
    case AST_STMT_USE:
        printf("use statement: " ANSI_BOLD_GREEN "{\n" ANSI_RESET);
        printer_do_indent();
        print_op_from_type(TOK_IMPORT);
        print_indent();
        printf("module name: ");
        print_id_slice(stmt->stmt.use.module_id);
        puts(",");
        printer_deindent();
        break;
    case AST_STMT_COMPT_MODIFIER:
        puts("comptime-declaration statement: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_delineator_from_type(TOK_COMPT);
        printer_do_indent();
        print_stmt(stmt->stmt.compt_modifier.stmt);
        printer_deindent();
        break;
    }
    print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_BOLD_BLUE ",\n" ANSI_RESET);
}
