//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/parser/parse_stmt.h"
#include "compiler/ast/expr.h"
#include "compiler/ast/printer.h"
#include "compiler/ast/stmt.h"
#include "compiler/parser/parse_expr.h"
#include "compiler/parser/parse_type.h"
#include "compiler/parser/parser.h"
#include "compiler/parser/token_eaters.h"
#include "compiler/token.h"
#include "utils/arena.h"
#include "utils/spill_arr.h"
#include "utils/vector.h"
#include <string.h>

/// right now this is just some placeholder logic to test the basic parser functions
/// no ast is being raised, we're just running through the tokens
/// def of this fn is lower down in this src file
void temp_eat(parser_t* p);

size_t parser_estimate_stmt_cnt(vector_t* tkn_vec) {
    // num stmts roughly <= 2 * num lines
    token_t* tkn = vector_last(tkn_vec);
    return 2 * (tkn->loc.line + 1); // + 1 since zero-indexed
}

ast_stmt_t* parse_file(parser_t* p, const char* file_name) {
    ast_stmt_t* file = parser_alloc_stmt(p);
    file->type = AST_STMT_FILE;
    file->stmt.file.file_name = file_name;
    file->stmt.file.stmts = parse_slice_of_stmts(p, TOK_EOF);
    if (file->stmt.file.stmts.len != 0) {
        file->first = file->stmt.file.stmts.start[0]->first;
        file->last = file->stmt.file.stmts.start[file->stmt.file.stmts.len - 1]->last;
    } else {
        // since eating never runs past eof, this is safe
        file->last = parser_eat(p);
        file->first = parser_eat(p);
    }
    return file;
}

ast_slice_of_stmts_t parse_slice_of_stmts(parser_t* p, token_type_e until_tkn) {
    spill_arr_ptr_t sarr = spill_arr_ptr_create();

    while (!(parser_peek(p)->type == until_tkn) && !parser_eof(p)) {
        spill_arr_ptr_push(&sarr, parse_stmt(p));
    }

    return parser_freeze_stmt_spill_arr(p, &sarr);
}

ast_slice_of_stmts_t parser_freeze_stmt_spill_arr(parser_t* p, spill_arr_ptr_t* sarr) {
    ast_slice_of_stmts_t slice = {
        .start = (ast_stmt_t**)arena_alloc(p->arena, sarr->size * sizeof(ast_stmt_t*)),
        .len = sarr->size,
    };
    spill_arr_ptr_flat_copy((void**)slice.start, sarr);
    spill_arr_ptr_destroy(sarr);
    return slice;
}

ast_slice_of_params_t parser_freeze_params_vec(parser_t* p, vector_t* vec) {
    ast_slice_of_params_t slice = {
        .start = (ast_param_t*)arena_alloc(p->arena, vec->size * vec->elem_size),
        .len = vec->size,
    };
    memcpy((void*)slice.start, vec->data, vec->size * vec->elem_size);
    vector_destroy(vec);
    return slice;
}

ast_stmt_t* parser_alloc_stmt(parser_t* p) { return arena_alloc(p->arena, sizeof(ast_stmt_t)); }

static ast_stmt_t* parser_sync_stmt(parser_t* p) {
    token_range_t range = parser_sync(p);
    ast_stmt_t* dummy_stmt = parser_alloc_stmt(p);
    dummy_stmt->type = AST_STMT_INVALID;
    dummy_stmt->first = range.first;
    dummy_stmt->last = range.last;
    return dummy_stmt;
}

ast_stmt_t* parse_stmt(parser_t* p) {
    token_t* first_tkn = parser_peek(p);
    token_type_e next_type = first_tkn->type;

    // parse things that lead with a token (easier)
    if (next_type == TOK_LBRACE) {
        return parse_stmt_block(p);
    }

    if (token_is_function_leading_kw(next_type)) {
        return parse_fn_decl(p);
    }

    if (next_type == TOK_MUT) {
        return parse_var_decl(p, NULL, true); // leading_mut = true
    }

    if (next_type == TOK_RETURN) {
        return parse_stmt_return(p);
    }

    // parse things that have a leading expr or identifier (trickier)
    ast_expr_t* leading_expr = parse_expr(p);
    next_type = parser_peek(p)->type;
    if (token_is_type_indicator(next_type) && leading_expr->type == AST_EXPR_ID) {
        return parse_var_decl(p, leading_expr, NULL); // leading expr as type
    }

    if (leading_expr->type == AST_EXPR_INVALID) {
        return parser_sync_stmt(p);
    }

    return parse_stmt_expr(p, leading_expr);
}

ast_stmt_t* parse_stmt_expr(parser_t* p, ast_expr_t* expr) {
    ast_stmt_t* stmt = parser_alloc_stmt(p);
    stmt->stmt.stmt_expr.expr = expr;
    token_t* term = parser_expect_token(p, TOK_SEMICOLON);
    if (!term) {
        return parser_sync_stmt(p);
    }
    stmt->stmt.stmt_expr.terminator = term;
    stmt->type = AST_STMT_EXPR;
    stmt->first = stmt->stmt.stmt_expr.expr->first;
    stmt->last = stmt->stmt.stmt_expr.terminator;
    return stmt;
}

ast_stmt_t* parse_stmt_block(parser_t* p) {
    ast_stmt_t* stmt = parser_alloc_stmt(p);
    stmt->type = AST_STMT_BLOCK;
    token_t* lbrace = parser_expect_token(p, TOK_LBRACE);
    if (!lbrace) {
        return parser_sync_stmt(p);
    }
    stmt->stmt.block.left_delim = lbrace;
    stmt->stmt.block.stmts = parse_slice_of_stmts(p, TOK_RBRACE);
    token_t* rbrace = parser_expect_token(p, TOK_RBRACE);
    if (!rbrace) {
        return parser_sync_stmt(p);
    }
    stmt->first = lbrace;
    stmt->last = rbrace;
    return stmt;
}

ast_stmt_t* parse_var_decl(parser_t* p, ast_expr_t* id_expr, bool leading_mut) {
    ast_stmt_t* stmt = parser_alloc_stmt(p);

    ast_type_t* type;
    if (leading_mut) {
        type = parse_type_with_leading_mut(p);
    } else {
        type = parse_type_with_leading_id(p, id_expr->expr.id.slice);
    }

    token_t* name = parse_var_name(p);
    token_type_e next_type = parser_peek(p)->type;

    if (next_type == TOK_SEMICOLON) {
        stmt->type = AST_STMT_VAR_DECL;
        stmt->stmt.var_decl.type = type;
        stmt->stmt.var_decl.name = name;
        stmt->stmt.var_decl.terminator = parser_expect_token(p, TOK_SEMICOLON);
    } else if (token_is_assignment_init(next_type)) {
        stmt->type = AST_STMT_VAR_INIT_DECL;
        stmt->stmt.var_init_decl.type = type;
        stmt->stmt.var_init_decl.name = name;
        // we already know next type is an assignment init token
        stmt->stmt.var_init_decl.assign_op = parser_eat(p);
        stmt->stmt.var_init_decl.rhs = parse_expr(p);
        stmt->stmt.var_init_decl.terminator = parser_expect_token(p, TOK_SEMICOLON);
    }
    stmt->first = type->first;
    stmt->last = stmt->stmt.var_decl.terminator;
    return stmt;
}

ast_stmt_t* parse_fn_decl(parser_t* p) {
    ast_stmt_t* decl = parser_alloc_stmt(p);
    decl->type = AST_STMT_FN_DECL;
    decl->stmt.fn_decl.kw = parser_eat(p); // fine because we knew to enter this function
    decl->stmt.fn_decl.name = parse_token_ptr_slice(p, TOK_SCOPE_RES);

    token_t* lparen = parser_expect_token(p, TOK_LPAREN);
    if (!lparen) {
        return parser_sync_stmt(p);
    }
    decl->stmt.fn_decl.left_paren = lparen;

    token_t* rparen = parser_expect_token(p, TOK_RPAREN);
    if (!rparen) {
        return parser_sync_stmt(p);
    }
    decl->stmt.fn_decl.right_paren = rparen;

    token_t* rarrow = parser_match_token(p, TOK_RARROW);
    if (rarrow) {
        decl->stmt.fn_decl.rarrow = rarrow;
        decl->stmt.fn_decl.return_type = parse_type(p);
    }
    decl->stmt.fn_decl.block = parse_stmt_block(p);

    decl->first = decl->stmt.fn_decl.kw;
    decl->last = decl->stmt.fn_decl.block->last;
    return decl;
}

ast_stmt_t* parse_stmt_return(parser_t* p) {
    ast_stmt_t* ret_stmt = parser_alloc_stmt(p);
    ret_stmt->type = AST_STMT_RETURN;
    ret_stmt->stmt.return_stmt.return_tkn =
        parser_eat(p); // safe becuz we knew to enter this function
    // check if we should parse an expr
    if (!(parser_peek(p)->type == TOK_SEMICOLON)) {
        ret_stmt->stmt.return_stmt.expr = parse_expr(p);
    }
    token_t* term = parser_expect_token(p, TOK_SEMICOLON);
    if (!term) {
        return parser_sync_stmt(p);
    }
    ret_stmt->stmt.return_stmt.terminator = term;
    ret_stmt->first = ret_stmt->stmt.return_stmt.return_tkn;
    ret_stmt->last = term;
    return ret_stmt;
}
