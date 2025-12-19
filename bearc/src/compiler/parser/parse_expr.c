//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/parser/parse_expr.h"
#include "compiler/ast/expr.h"
#include "compiler/diagnostics/error_codes.h"
#include "compiler/parser/rules.h"
#include "compiler/parser/token_eaters.h"
#include "compiler/token.h"
#include "parse_token_slice.h"
#include "utils/arena.h"
#include "utils/spill_arr.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define PREC_INIT UINT8_MAX

ast_slice_of_exprs_t parser_freeze_expr_spill_arr(parser_t* p, spill_arr_ptr_t* sarr) {
    ast_slice_of_exprs_t slice = {
        .start = (ast_expr_t**)arena_alloc(p->arena, sarr->size * sizeof(ast_expr_t*)),
        .len = sarr->size,
    };
    memcpy((void*)slice.start, (void*)sarr->data, sarr->size * sizeof(ast_expr_t*));
    spill_arr_ptr_destroy(sarr);
    return slice;
}

ast_expr_t* parser_alloc_expr(parser_t* p) { return arena_alloc(p->arena, sizeof(ast_expr_t)); }

ast_expr_t* parse_expr(parser_t* p) {
    ast_expr_t* lhs = parse_primary_expr(p);
    return parse_expr_prec(p, lhs, PREC_INIT);
}

ast_expr_t* parse_expr_prec(parser_t* p, ast_expr_t* lhs, uint8_t prec) {
    token_type_e op = parser_peek(p)->type;
    if (!lhs && is_preunary_op(op)) {
        return parse_preunary_expr(p);
    }
    if (is_binary_op(parser_peek(p)->type)) {
        return parse_binary(p, lhs, prec);
    }
    if (!lhs) {
        return parse_expr(p);
    }
    assert(lhs != NULL && "[parse_expr.c|parse_expr_prec] lhs is NULL");
    return lhs;
}

ast_expr_t* parse_primary_expr(parser_t* p) {
    token_t* first_tkn = parser_peek(p);
    token_type_e first_type = first_tkn->type;
    ast_expr_t* lhs = NULL;
    // try to parse atoms as lhs ~~~~~~~~~~~~~~~~~~~
    if (token_is_builtin_type_or_id(first_type)) {
        lhs = parse_id(p);
    } else if (token_is_literal(first_type)) {
        lhs = parse_literal(p);
    } else if (first_type == TOK_LPAREN) {
        lhs = parse_grouping(p);
    }
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // if atom, try to parse postunary, since postunary prec > preunary prec
    if (lhs && is_postunary_op(parser_peek(p)->type)) {
        return parse_postunary(p, lhs);
    }
    // try fn call too
    if (lhs && parser_peek(p)->type == TOK_LPAREN) {
        return parse_fn_call(p, lhs);
    }
    // try subscript
    if (lhs && parser_peek(p)->type == TOK_LBRACK) {
        return parse_subscript(p, lhs);
    }
    if (is_preunary_op(first_type)) {
        return parse_expr_prec(p, NULL, PREC_INIT);
    }
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (lhs) {
        return lhs;
    }
    // failure case
    compiler_error_list_emplace(p->error_list, first_tkn, ERR_EXPECTED_EXPRESSION);
    return parser_sync_expr(p);
}

ast_expr_t* parse_preunary_expr(parser_t* p) {
    ast_expr_t* preunary_expr = parser_alloc_expr(p);
    token_t* op = parser_eat(p); // already been checked that this token is legit
    // set op
    preunary_expr->type = AST_EXPR_PRE_UNARY;
    preunary_expr->first = op;
    preunary_expr->expr.unary.op = op;
    // get and set sub expression
    ast_expr_t* sub_expr = parse_expr_prec(p, NULL, prec_preunary(op->type));
    preunary_expr->expr.unary.expr = sub_expr;
    preunary_expr->last = sub_expr->last;
    return preunary_expr;
}

ast_expr_t* parse_literal(parser_t* p) {
    ast_expr_t* lit_expr = parser_alloc_expr(p);
    token_t* tkn = parser_eat(p);
    lit_expr->expr.literal.tkn = tkn;
    lit_expr->type = AST_EXPR_LITERAL;
    lit_expr->first = tkn;
    lit_expr->last = tkn;
    return lit_expr;
}

ast_expr_t* parse_id(parser_t* p) {
    ast_expr_t* id_expr = parser_alloc_expr(p);
    id_expr->expr.id.slice = parse_token_ptr_slice(p, TOK_SCOPE_RES);
    id_expr->type = AST_EXPR_ID;
    id_expr->first = id_expr->expr.id.slice.start[0];
    id_expr->last = id_expr->expr.id.slice.start[id_expr->expr.id.slice.len - 1];
    return id_expr;
}

token_t* parse_var_name(parser_t* p) {
    return parser_expect_token_with_err_code(p, TOK_IDENTIFIER, ERR_ILLEGAL_IDENTIFER);
}

static bool binary_bind_right(token_type_e curr_op, token_type_e next_op) {
    return (is_binary_op(next_op) && (prec_binary(next_op) < prec_binary(curr_op))) ||
           (is_binary_op(next_op) && (prec_binary(next_op) < prec_binary(curr_op)) &&
            is_right_assoc_from_prec(prec_binary(curr_op)));
}

ast_expr_t* parse_binary(parser_t* p, ast_expr_t* lhs, uint8_t max_prec) {
    ast_expr_t* binary_expr = parser_alloc_expr(p);
    token_t* op_tkn = parser_eat(p); // already verfied legit
    binary_expr->type = AST_EXPR_BINARY;
    binary_expr->expr.binary.lhs = lhs;
    binary_expr->expr.binary.op = op_tkn;
    max_prec = (max_prec >= prec_binary(op_tkn->type)) ? max_prec : prec_binary(op_tkn->type);
    ast_expr_t* rhs = parse_primary_expr(p);
    token_type_e curr_op = op_tkn->type;
    token_type_e next_op = parser_peek(p)->type;

    while (binary_bind_right(curr_op, next_op)) {
        rhs = parse_expr_prec(p, rhs, prec_binary(next_op));
        curr_op = next_op, next_op = parser_peek(p)->type;
    }
    binary_expr->expr.binary.rhs = rhs;
    binary_expr->first = binary_expr->expr.binary.lhs->first;
    binary_expr->last = binary_expr->expr.binary.rhs->last;
    return parse_expr_prec(p, binary_expr, prec_binary(curr_op));
}

ast_expr_t* parse_postunary(parser_t* p, ast_expr_t* lhs) {
    ast_expr_t* postunary_expr = parser_alloc_expr(p);
    token_t* op = parser_eat(p); // already verfied legit
    postunary_expr->type = AST_EXPR_POST_UNARY;
    postunary_expr->expr.unary.expr = lhs;
    postunary_expr->expr.unary.op = op;
    postunary_expr->first = postunary_expr->expr.unary.expr->first;
    postunary_expr->last = op;
    return postunary_expr;
}

ast_expr_t* parse_fn_call(parser_t* p, ast_expr_t* lhs) {
    ast_expr_t* call_expr = parser_alloc_expr(p);
    token_t* lparen = parser_eat(p); // already verfied legit
    call_expr->type = AST_EXPR_FN_CALL;
    call_expr->expr.fn_call.left_expr = lhs;
    call_expr->expr.fn_call.left_paren = lparen;

    spill_arr_ptr_t args = spill_arr_ptr_create();

    // skip the args parsing loop if the struct is foo()
    if (!(parser_peek(p)->type == TOK_RPAREN)) {
        do {
            *((ast_expr_t**)spill_arr_ptr_emplace(&args)) = parse_expr(p);
        } while (parser_match_token(p, TOK_COMMA));
    }

    token_t* rparen = parser_expect_token(p, TOK_RPAREN);
    if (!rparen) {
        return parser_sync_expr(p);
    }
    call_expr->expr.fn_call.right_paren = rparen;

    call_expr->expr.fn_call.args = parser_freeze_expr_spill_arr(p, &args);

    call_expr->first = call_expr->expr.fn_call.left_expr->first;
    call_expr->last = rparen;
    return call_expr;
}

ast_expr_t* parser_sync_expr(parser_t* p) {
    token_range_t range = parser_sync(p);
    ast_expr_t* dummy_expr = parser_alloc_expr(p);
    dummy_expr->type = AST_EXPR_INVALID;
    dummy_expr->first = range.first;
    dummy_expr->last = range.last;
    return dummy_expr;
}

ast_expr_t* parse_grouping(parser_t* p) {
    ast_expr_t* grouping = parser_alloc_expr(p);
    grouping->type = AST_EXPR_GROUPING;
    token_t* lparen = parser_eat(p);
    grouping->expr.grouping.left_paren = lparen;
    grouping->expr.grouping.expr = parse_expr(p);
    token_t* rparen = parser_expect_token(p, TOK_RPAREN);
    if (!rparen) {
        return parser_sync_expr(p);
    }

    grouping->expr.grouping.right_paren = rparen;
    grouping->first = lparen;
    grouping->last = grouping->expr.grouping.right_paren;
    return grouping;
}

ast_expr_t* parse_subscript(parser_t* p, ast_expr_t* lhs) {
    ast_expr_t* s = parser_alloc_expr(p);
    s->type = AST_EXPR_SUBSCRIPT;
    s->expr.subscript.lhs = lhs;
    token_t* lbrack = parser_expect_token(p, TOK_LBRACK);
    if (!lbrack) {
        return parser_sync_expr(p);
    }
    s->expr.subscript.subexpr = parse_expr(p);
    token_t* rbrack = parser_expect_token(p, TOK_RBRACK);
    if (!rbrack) {
        return parser_sync_expr(p);
    }
    s->first = s->expr.subscript.lhs->first;
    s->last = rbrack;
    return s;
}
