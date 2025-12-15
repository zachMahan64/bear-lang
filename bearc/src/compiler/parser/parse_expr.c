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
    token_type_e tkn_type = first_tkn->type;
    if (token_is_literal(tkn_type)) {
        return parse_literal(p);
    }
    if (token_is_builtin_type_or_id(tkn_type) || tkn_type == TOK_IDENTIFIER) {
        return parse_id(p);
    }

    // ++, etc.
    if (tkn_type == TOK_PLUS || tkn_type == TOK_MINUS || tkn_type == TOK_DEC ||
        tkn_type == TOK_INC || tkn_type == TOK_BIT_NOT || tkn_type == TOK_BOOL_NOT) {
        return parse_preunary_expr(p);
    }
    // complete failure case
    compiler_error_list_emplace(p->error_list, first_tkn, ERR_EXPECTED_EXPRESSION);
    return NULL;
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
