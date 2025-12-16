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
#include "compiler/parser/token_eaters.h"
#include "compiler/token.h"
#include "utils/arena.h"
#include "utils/vector.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

token_ptr_slice_t parser_freeze_token_ptr_slice(parser_t* p, vector_t* vec) {
    token_ptr_slice_t slice = {
        .start = (token_t**)arena_alloc(p->arena, vec->size * vec->elem_size),
        .len = vec->size,
    };
    memcpy((void*)slice.start, vec->data, vec->size * vec->elem_size);
    vector_destroy(vec);
    return slice;
}

ast_slice_of_exprs_t parser_freeze_expr_vec(parser_t* p, vector_t* vec) {
    ast_slice_of_exprs_t slice = {
        .start = arena_alloc(p->arena, vec->size * vec->elem_size),
        .len = vec->size,
    };
    memcpy(slice.start, vec->data, vec->size * vec->elem_size);
    vector_destroy(vec);
    return slice;
}

ast_expr_t* parser_alloc_expr(parser_t* p) { return arena_alloc(p->arena, sizeof(ast_expr_t)); }

ast_expr_t* parse_expr(parser_t* p) {

    token_t* first_tkn = parser_peek(p);
    token_type_e first_type = first_tkn->type;

    ast_expr_t* lhs;
    if (token_is_builtin_type_or_id(first_type) || first_type == TOK_IDENTIFIER) {
        lhs = parse_id(p);
        if (token_is_binary_op(parser_peek(p)->type)) {
            return parse_binary(p, lhs);
        }
        return lhs;
    }
    if (token_is_literal(first_type)) {
        lhs = parse_literal(p);
        if (token_is_binary_op(parser_peek(p)->type)) {
            return parse_binary(p, lhs);
        }
        return lhs;
    }
    if (first_type == TOK_LPAREN) {
        return parse_grouping(p);
    }
    if (token_is_preunary_op(first_type)) {
        lhs = parse_preunary_expr(p);
        if (token_is_binary_op(parser_peek(p)->type)) {
            return parse_binary(p, lhs);
        }
        return lhs;
    }
    // complete failure case
    compiler_error_list_emplace(p->error_list, first_tkn, ERR_EXPECTED_EXPRESSION);
    // printf("%.*s\n", (int)first_tkn->length, first_tkn->start);

    return parser_sync(p);
}

ast_expr_t* parse_preunary_expr(parser_t* p) {
    ast_expr_t* preunary_expr = parser_alloc_expr(p);
    token_t* op = parser_eat(p); // already been checked that this token is legit
    // set op
    preunary_expr->type = AST_EXPR_PRE_UNARY;
    preunary_expr->first = op;
    preunary_expr->expr.unary.op = op;
    // get and set sub expression
    ast_expr_t* sub_expr = parse_expr(p);
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
#define PARSER_EXPR_ID_VEC_CAP 16 // pretty safe
    ast_expr_t* id_expr = parser_alloc_expr(p);
    vector_t id_vec = vector_create_and_reserve(sizeof(token_t*), PARSER_EXPR_ID_VEC_CAP);
    token_type_e type;
    while ((type = token_is_builtin_type_or_id(parser_peek(p)->type)) || (type == TOK_SCOPE_RES)) {
        if (type == TOK_SCOPE_RES) {
            parser_eat(p);
        } else {
            vector_push_back(&id_vec, parser_eat(p));
        }
    }
    id_expr->type = AST_EXPR_ID;
    id_expr->expr.id.slice = parser_freeze_token_ptr_slice(p, &id_vec);
    id_expr->first = id_expr->expr.id.slice.start[0];
    id_expr->last = id_expr->expr.id.slice.start[id_expr->expr.id.slice.len - 1];
    return id_expr;
}

ast_expr_t* parse_binary(parser_t* p, ast_expr_t* lhs) {
    /// TODO handle precedence
    ast_expr_t* binary_expr = parser_alloc_expr(p);
    token_t* op = parser_eat(p); // already verfied legit
    binary_expr->type = AST_EXPR_BINARY;
    binary_expr->expr.binary.lhs = lhs;
    binary_expr->expr.binary.op = op;
    binary_expr->expr.binary.rhs = parse_expr(p);
    binary_expr->first = binary_expr->expr.binary.lhs->first;
    binary_expr->last = binary_expr->expr.binary.rhs->last;
    return binary_expr;
}

ast_expr_t* parser_sync(parser_t* p) {
    token_t* first_tkn = parser_eat(p);
    token_t* last_tkn = first_tkn; // init in case loop never runs!
    while (!parser_eof(p)) {
        token_t* curr = parser_eat(p);
        token_type_e t = curr->type;
        if (t == TOK_SEMICOLON || t == TOK_LBRACE || t == TOK_RBRACE) {
            last_tkn = curr;
            break;
        }
    }
    ast_expr_t* dummy_expr = parser_alloc_expr(p);
    dummy_expr->type = AST_INVALID;
    dummy_expr->first = first_tkn;
    dummy_expr->last = last_tkn;
    return dummy_expr;
}

ast_expr_t* parse_grouping(parser_t* p) {
    ast_expr_t* grouping = parser_alloc_expr(p);
    token_t* lparen = parser_eat(p);
    grouping->expr.grouping.left_paren = lparen;
    grouping->expr.grouping.expr = parse_expr(p);
    grouping->expr.grouping.right_paren = parser_expect_token(p, TOK_RPAREN);
    grouping->first = lparen;
    grouping->last = grouping->expr.grouping.right_paren;
    return grouping;
}
