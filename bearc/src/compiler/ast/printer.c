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
        printf("%smissing tkn%s", ansi_bold_red(), ansi_reset());
        return;
    }
    printf("%.*s", (int)tkn->len, tkn->start);
}

static void print_op(token_t* op) {
    printer_do_indent(), print_indent(), printf("%s`%s", ansi_bold_green(), ansi_bold_magenta()),
        print_tkn(op), printf("%s`%s,\n", ansi_bold_green(), ansi_reset()), printer_deindent();
}

static void print_var_name(token_t* name) {
    printer_do_indent(), print_indent(), printf("name: %s`%s", ansi_bold_green(), ansi_bold_cyan()),
        print_tkn(name), printf("%s`%s,\n", ansi_bold_green(), ansi_reset()), printer_deindent();
}

static void print_comma(void) {
    printer_do_indent(), print_indent(),
        printf("%s`%s,%s`%s,\n", ansi_bold_green(), ansi_bold_yellow(), ansi_bold_green(),
               ansi_reset());
    printer_deindent();
}

static void print_opening_delim(token_t* delim) {
    printer_do_indent(), print_indent(), printf("%s`%s", ansi_bold_green(), ansi_bold_yellow()),
        print_tkn(delim), printf("%s`%s,\n", ansi_bold_green(), ansi_reset());
}

static void print_closing_delim(token_t* delim) {
    print_indent(), printf("%s`%s", ansi_bold_green(), ansi_bold_yellow()), print_tkn(delim),
        printf("%s`%s,\n", ansi_bold_green(), ansi_reset());
    printer_deindent();
}

static void print_opening_delim_from_type(token_type_e delim) {
    printer_do_indent(), print_indent(),
        printf("%s`%s%s", ansi_bold_green(), ansi_bold_yellow(), token_to_string_map()[delim]),
        printf("%s`%s,\n", ansi_bold_green(), ansi_reset());
}

static void print_closing_delim_from_type(token_type_e delim) {
    print_indent(),
        printf("%s`%s%s", ansi_bold_green(), ansi_bold_yellow(), token_to_string_map()[delim]),
        printf("%s`%s,\n", ansi_bold_green(), ansi_reset()), printer_deindent();
}

static void print_delineator_from_type(token_type_e delin) {
    printer_do_indent();
    print_indent(),
        printf(ANSI_BOLD_GREEN "`" ANSI_RESET ANSI_BOLD_YELLOW "%s", token_to_string_map()[delin]),
        puts(ANSI_BOLD_GREEN "`" ANSI_RESET ",");
    printer_deindent();
}

static void print_terminator(token_t* term) {
    if (!term) {
        print_indent(), printf("%smissing terminator%s\n", ansi_bold_red(), ansi_reset());
        return;
    }
    print_indent(), printf("%s`%s", ansi_bold_green(), ansi_bold_yellow()), print_tkn(term),
        printf("%s`%s,\n", ansi_bold_green(), ansi_reset());
}

static void print_op_from_type(token_type_e t) {
    print_indent(), printf("%s`%s%s%s`%s,\n", ansi_bold_green(), ansi_bold_magenta(),
                           token_to_string_map()[t], ansi_bold_green(), ansi_reset());
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
        print_indent(), printf("%sinvalid generic arg%s,\n", ansi_bold_red(), ansi_reset());
        printer_deindent();
    }
}

static void print_title(const char* title) {
    printf("%s: %s{%s\n", title, ansi_bold_green(), ansi_reset());
}

static void print_closing_green_brace(void) {
    print_indent(), printf("%s}%s", ansi_bold_green(), ansi_reset());
}

static void print_closing_green_brace_newline(void) {
    print_indent(), printf("%s}%s\n", ansi_bold_green(), ansi_reset());
}

static void print_type(ast_type_t* type) {
    printer_do_indent();
    print_indent();

    switch (type->tag) {
    case AST_TYPE_BASE: {
        token_ptr_slice_t ids = type->type.base.id;
        print_title("base type");
        printer_do_indent();
        print_indent();
        printf(ANSI_BOLD_GREEN "`" ANSI_RESET);
        for (size_t i = 0; i < ids.len; i++) {
            int len = (int)ids.start[i]->len;
            const char* start = ids.start[i]->start;
            printf("%s%.*s%s", ansi_bold_yellow(), len, start, ansi_reset());
            if (ids.len != 1 && i != ids.len - 1) {
                printf("%s%s%s", ansi_bold_green(), token_to_string_map()[TOK_SCOPE_RES],
                       ansi_bold_yellow());
            }
        }
        printf("%s`%s", ansi_bold_green(), ansi_reset());
        printf(",\n");
        if (type->type.base.mut) {
            print_mut();
        }
        printer_deindent();
        print_closing_green_brace();
        break;
    }
    case AST_TYPE_REF_PTR: {
        print_title("ref/ptr type");
        print_type(type->type.ref.inner);
        print_op(type->type.ref.modifier);
        if (type->type.ref.mut) {
            printer_do_indent();
            print_mut();
            printer_deindent();
        }
        print_closing_green_brace();
        break;
    }
    case AST_TYPE_ARR:
        print_title("array type");
        print_opening_delim_from_type(TOK_LBRACK);
        print_expr(type->type.arr.size_expr);
        print_closing_delim_from_type(TOK_RBRACK);
        print_type(type->type.arr.inner);
        print_closing_green_brace();
        break;
    case AST_TYPE_GENERIC:
        print_title("generic type");
        print_type(type->type.generic.inner);
        print_delineator_from_type(TOK_GENERIC_SEP);
        print_opening_delim_from_type(TOK_LT);
        for (size_t i = 0; i < type->type.generic.generic_args.len; i++) {
            print_generic_type_arg(type->type.generic.generic_args.start[i]);
        }
        print_closing_delim_from_type(TOK_GT);
        print_closing_green_brace();
        break;
    case AST_TYPE_INVALID:
        print_indent();
        printf("%sinvalid type%s", ansi_bold_red(), ansi_reset());
        break;
    case AST_TYPE_SLICE:
        print_title("slice type");
        print_opening_delim_from_type(TOK_LBRACK);
        print_op_from_type(TOK_AMPER);
        if (type->type.slice.mut) {
            print_mut();
        }
        print_closing_delim_from_type(TOK_RBRACK);
        print_type(type->type.slice.inner);
        print_closing_green_brace();
        break;
    case AST_TYPE_FN_PTR: {
        print_title("function-ptr type");
        print_delineator_from_type(TOK_STAR);
        print_delineator_from_type(TOK_FN);
        print_opening_delim_from_type(TOK_LPAREN);
        ast_slice_of_types_t types = type->type.fn_ptr.param_types;
        for (size_t i = 0; i < types.len; i++) {
            print_type(types.start[i]);
        }
        print_closing_delim_from_type(TOK_RPAREN);
        if (type->type.fn_ptr.return_type) {
            print_delineator_from_type(TOK_RARROW);
            print_type(type->type.fn_ptr.return_type);
        }
        print_closing_green_brace();
        break;
    }
    }
    puts(",");
    printer_deindent();
}

static void print_param(ast_param_t* param) {
    print_indent();
    if (!param->valid) {
        printf("%sinvalid parameter,\n%s", ansi_bold_red(), ansi_reset());
        return;
    }
    print_title("parameter");
    print_type(param->type);
    print_var_name(param->name);
    print_closing_green_brace();
    puts(",");
}

static void print_id_slice(token_ptr_slice_t ids) {
    printer_do_indent();
    printf(ANSI_BOLD_GREEN "`" ANSI_RESET);
    for (size_t i = 0; i < ids.len; i++) {
        int len = (int)ids.start[i]->len;
        const char* start = ids.start[i]->start;
        printf("%s%.*s%s", ansi_bold_cyan(), len, start, ansi_reset());
        if (ids.len != 1 && i != ids.len - 1) {
            printf("%s%s%s", ansi_bold_green(), token_to_string_map()[TOK_SCOPE_RES], ansi_reset());
        }
    }
    printf("%s`%s", ansi_bold_green(), ansi_reset());
    printer_deindent();
}

static void print_id_slice_name(token_ptr_slice_t id) {
    printer_do_indent();
    print_indent(), printf("name: "), print_id_slice(id), printf(",\n");
    printer_deindent();
}

static void print_id_with_contracts(ast_type_with_contracts_t* t) {
    print_indent();
    if (!t->valid) {
        printf("%sinvalid parameter,%s\n", ansi_bold_red(), ansi_reset());
        return;
    }
    print_title("type-name");
    print_var_name(t->id);
    if (t->contract_ids.len != 0) {
        printer_do_indent();
        print_op_from_type(TOK_HAS);
        printer_deindent();
        print_opening_delim_from_type(TOK_LPAREN);
        for (size_t i = 0; i < t->contract_ids.len; i++) {
            print_expr(t->contract_ids.start[i]);
        }
        print_closing_delim_from_type(TOK_RPAREN);
    }
    print_closing_green_brace_newline();
}

static void print_generic_params(ast_slice_of_generic_params_t params) {
    print_indent();
    print_title("generic parameter list");
    printer_do_indent();
    for (size_t i = 0; i < params.len; i++) {
        switch (params.start[i]->tag) {
        case AST_GENERIC_PARAM_TYPE:
            print_id_with_contracts(params.start[i]->param.generic_type);
            break;
        case AST_GENERIC_PARAM_VAR:
            print_param(params.start[i]->param.generic_var);
            break;
        case AST_GENERIC_PARAM_INVALID:
            print_indent(), printf("%sinvalid generic param%s,\n", ansi_bold_red(), ansi_reset());
            break;
        }
    }
    printer_deindent();
    print_closing_green_brace_newline();
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
        const char* lit_type_str = token_to_string_map()[tkn->type];
        printf("literal (%s): %s`%s%.*s%s`%s", lit_type_str, ansi_bold_green(), ansi_bold_blue(),
               (int)tkn->len, tkn->start, ansi_bold_green(), ansi_reset());
        break;
    }
    case AST_EXPR_BINARY: {
        print_title("binary-expr");
        ast_expr_t* lhs = expr.expr.binary.lhs;
        token_t* op = expr.expr.binary.op;
        ast_expr_t* rhs = expr.expr.binary.rhs;
        print_expr(lhs);
        print_op(op);
        print_expr(rhs);
        print_closing_green_brace();
        break;
    }
    case AST_EXPR_GROUPING: {
        print_title("grouping");
        print_expr(expr.expr.grouping.expr);
        print_closing_green_brace();
        break;
    }
    case AST_EXPR_PRE_UNARY: {
        token_t* op = expr.expr.unary.op;
        print_title("pre-unary");
        print_op(op);
        print_expr(expr.expr.unary.expr);
        print_closing_green_brace();
        break;
    }
    case AST_EXPR_POST_UNARY: {
        token_t* op = expr.expr.unary.op;
        print_title("post-unary");
        print_expr(expr.expr.unary.expr);
        print_op(op);
        print_closing_green_brace();
        break;
    }
    case AST_EXPR_FN_CALL:
        print_title("function call");
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
        print_closing_green_brace();
        break;
    case AST_EXPR_INVALID:
        printf("%sinvalid expression%s", ansi_bold_red(), ansi_reset());
        break;
        // TODO resume here
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
    case AST_EXPR_STRUCT_INIT: {
        puts("struct-init expression: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_indent();
        printf("name: ");
        print_id_slice(expr.expr.struct_init.id);
        puts(",");
        print_delineator_from_type(TOK_LBRACE);
        ast_slice_of_exprs_t inits = expr.expr.struct_init.member_inits;
        for (size_t i = 0; i < inits.len; i++) {
            print_expr(inits.start[i]);
        }
        print_delineator_from_type(TOK_RBRACE);
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    }
    case AST_EXPR_STRUCT_MEMBER_INIT: {
        puts("member-init: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_opening_delim(expr.expr.struct_member_init.id);
        printer_deindent();
        token_t* op = expr.expr.struct_member_init.assign_op;
        ast_expr_t* val = expr.expr.struct_member_init.value;
        print_op(op);
        print_expr(val);
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    }
    case AST_EXPR_BORROW: {
        puts("borrow: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        printer_do_indent();
        print_op_from_type(TOK_AMPER);
        if (expr.expr.borrow.mut) {
            print_op_from_type(TOK_MUT);
        }
        printer_deindent();
        print_expr(expr.expr.borrow.borrowed);
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    }
    case AST_EXPR_VARIANT_DECOMP: {
        puts("variant decomposition: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        ast_expr_variant_decomp_t vd = expr.expr.variant_decomp;
        print_id_slice_name(vd.id);
        if (vd.vars.len > 0) {
            print_opening_delim_from_type(TOK_LPAREN);
            for (size_t i = 0; i < vd.vars.len; i++) {
                print_param(vd.vars.start[i]);
            }
            print_closing_delim_from_type(TOK_RPAREN);
        }
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    }
    case AST_EXPR_BLOCK: {
        puts("block-expr: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_delineator_from_type(TOK_LBRACE);
        printer_do_indent();
        for (size_t i = 0; i < expr.expr.block.stmts.len; i++) {
            print_stmt(expr.expr.block.stmts.start[i]);
        }
        printer_deindent();
        print_delineator_from_type(TOK_RBRACE);
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    }
    case AST_EXPR_SWITCH_BRANCH: {
        puts("switch-branch-expr: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        ast_slice_of_exprs_t patterns = expr.expr.switch_branch.patterns;
        for (size_t i = 0; i < patterns.len; i++) {
            print_expr(patterns.start[i]);
            if (i != patterns.len - 1) {
                print_delineator_from_type(TOK_BAR);
            }
        }
        print_delineator_from_type(TOK_EQ_ARROW);
        print_expr(expr.expr.switch_branch.value);
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    }
    case AST_EXPR_SWITCH: {
        puts("switch-expr: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_op_from_type(TOK_SWITCH);
        print_delineator_from_type(TOK_LPAREN);
        print_expr(expr.expr.switch_expr.matched);
        print_delineator_from_type(TOK_RPAREN);
        ast_slice_of_exprs_t branches = expr.expr.switch_expr.branches;
        for (size_t i = 0; i < branches.len; i++) {
            print_expr(branches.start[i]);
        }
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    }
    case AST_EXPR_DEFAULT:
        printf(ANSI_BOLD_BLUE "%s" ANSI_RESET, token_to_string_map()[TOK_DEFAULT]);
        break;
    case AST_EXPR_CLOSURE: {
        puts("closure: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        ast_expr_closure_t cl = expr.expr.closure;
        print_opening_delim_from_type(TOK_BAR);
        for (size_t i = 0; i < cl.params.len; i++) {
            print_param(cl.params.start[i]);
        }
        print_closing_delim_from_type(TOK_BAR);
        print_expr(cl.body);
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_RESET);
        break;
    }
    }
    puts(",");
    printer_deindent();
}

void print_stmt(ast_stmt_t* stmt) {
    printer_try_init();
    print_indent();
    switch (stmt->type) {
    case AST_STMT_BLOCK:
        printf("block statement: " ANSI_BOLD_GREEN "{\n" ANSI_RESET);
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
        printer_deindent();
        break;
    case AST_STMT_EXPR:
        puts("expression-statement: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        print_expr(stmt->stmt.stmt_expr.expr);
        print_delineator_from_type(TOK_SEMICOLON);
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
                printf(ANSI_BOLD_GREEN "%s" ANSI_RESET, token_to_string_map()[TOK_SCOPE_RES]);
            }
        }
        printf(ANSI_BOLD_GREEN "`" ANSI_RESET);
        printf(",\n");
        if (fn.is_generic) {
            print_generic_params(fn.generic_params);
        }
        printer_deindent();
        print_opening_delim_from_type(TOK_LPAREN);
        for (size_t i = 0; i < fn.params.len; i++) {
            print_param(fn.params.start[i]);
        }
        print_closing_delim_from_type(TOK_RPAREN);
        if (fn.return_type) {
            printer_do_indent();
            print_op_from_type(TOK_RARROW);
            printer_deindent();
            print_type(fn.return_type);
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
        print_op_from_type(TOK_RETURN);
        ast_expr_t* expr = stmt->stmt.return_stmt.expr;
        if (expr) {
            print_expr(expr);
        }
        print_delineator_from_type(TOK_SEMICOLON);
        break;
    case AST_STMT_STRUCT_DEF:
        puts("struct declaration: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        ast_stmt_struct_decl_t st = stmt->stmt.struct_decl;
        print_delineator_from_type(TOK_STRUCT);
        printer_do_indent();
        print_id_with_contracts(st.name_with_contracts);
        if (st.is_generic) {
            print_generic_params(st.generic_params);
        }
        print_indent();
        puts("fields: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        printer_do_indent();
        for (size_t i = 0; i < st.fields.len; i++) {
            print_stmt(st.fields.start[i]);
        }
        printer_deindent();
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_BOLD_BLUE ",\n" ANSI_RESET);
        printer_deindent();
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
    case AST_STMT_FN_PROTOTYPE: {
        puts("function prototype: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        ast_stmt_fn_prototype_t fd = stmt->stmt.fn_prototype;
        print_op(fd.kw);
        printer_do_indent();
        print_indent();
        print_id_slice(fd.name);
        printf(",\n");
        if (fd.is_generic) {
            print_generic_params(fd.generic_params);
        }
        printer_deindent();
        print_opening_delim_from_type(TOK_LPAREN);
        for (size_t i = 0; i < fd.params.len; i++) {
            print_param(fd.params.start[i]);
        }
        print_closing_delim_from_type(TOK_RPAREN);
        if (fd.return_type) {
            printer_do_indent();
            print_op_from_type(TOK_RARROW);
            printer_deindent();
            print_type(fd.return_type);
        }
        print_delineator_from_type(TOK_SEMICOLON);
        break;
    }
    case AST_STMT_CONTRACT_DEF: {
        puts("contract declaration: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        ast_stmt_contract_decl_t con = stmt->stmt.contract_decl;
        print_delineator_from_type(TOK_CONTRACT);
        print_var_name(con.name);
        printer_do_indent();
        print_indent();
        puts("fields: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        printer_do_indent();
        for (size_t i = 0; i < con.fields.len; i++) {
            print_stmt(con.fields.start[i]);
        }
        printer_deindent();
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_BOLD_BLUE ",\n" ANSI_RESET);
        printer_deindent();
        break;
    }
    case AST_STMT_UNION_DEF: {
        puts("union declaration: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        ast_stmt_union_decl_t un = stmt->stmt.union_decl;
        print_delineator_from_type(TOK_UNION);
        print_var_name(un.name);
        printer_do_indent();
        print_indent();
        puts("fields: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        printer_do_indent();
        for (size_t i = 0; i < un.fields.len; i++) {
            print_stmt(un.fields.start[i]);
        }
        printer_deindent();
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_BOLD_BLUE ",\n" ANSI_RESET);
        printer_deindent();
        break;
    }
    case AST_STMT_VARIANT_DEF: {
        puts("variant declaration: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        ast_stmt_variant_decl_t vari = stmt->stmt.variant_decl;
        print_delineator_from_type(TOK_VARIANT);
        print_var_name(vari.name);
        printer_do_indent();
        if (vari.is_generic) {
            print_generic_params(vari.generic_params);
        }
        print_indent();
        puts("fields: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        printer_do_indent();
        for (size_t i = 0; i < vari.fields.len; i++) {
            print_stmt(vari.fields.start[i]);
        }
        printer_deindent();
        print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_BOLD_BLUE ",\n" ANSI_RESET);
        printer_deindent();
        break;
    }
    case AST_STMT_VARIANT_FIELD_DECL: {
        puts("variant field: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        ast_stmt_variant_field_decl_t fd = stmt->stmt.variant_field_decl;
        print_var_name(fd.name);
        if (fd.params.len > 0) {
            print_opening_delim_from_type(TOK_LPAREN);
            for (size_t i = 0; i < fd.params.len; i++) {
                print_param(fd.params.start[i]);
            }
            print_closing_delim_from_type(TOK_RPAREN);
        }
        break;
    }
    case AST_STMT_YIELD: {
        puts("yield statement: " ANSI_BOLD_GREEN "{" ANSI_RESET);
        printer_do_indent();
        print_op_from_type(TOK_YIELD);
        printer_deindent();
        ast_expr_t* expr = stmt->stmt.yield_stmt.expr;
        if (expr) {
            print_expr(expr);
        }
        print_delineator_from_type(TOK_SEMICOLON);
        break;
    }
    }
    print_indent(), printf(ANSI_BOLD_GREEN "}" ANSI_BOLD_BLUE ",\n" ANSI_RESET);
}
