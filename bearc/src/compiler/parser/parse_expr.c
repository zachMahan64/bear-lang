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
#include "compiler/diagnostics/error_list.h"
#include "compiler/parser/parse_type.h"
#include "compiler/parser/parser.h"
#include "compiler/parser/rules.h"
#include "compiler/parser/token_eaters.h"
#include "compiler/token.h"
#include "parse_token_slice.h"
#include "utils/arena.h"
#include "utils/spill_arr.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define PREC_INIT UINT8_MAX

ast_slice_of_exprs_t parser_freeze_expr_spill_arr(parser_t* p, spill_arr_ptr_t* sarr) {
    ast_slice_of_exprs_t slice = {
        .start = (ast_expr_t**)arena_alloc(p->arena, sarr->size * sizeof(ast_expr_t*)),
        .len = sarr->size,
    };
    spill_arr_ptr_flat_copy((void**)slice.start, sarr);
    spill_arr_ptr_destroy(sarr);
    return slice;
}

ast_slice_of_exprs_t parse_slice_of_exprs_call(parser_t* p, token_type_e divider,
                                               token_type_e until_tkn,
                                               ast_expr_t* (*call)(parser_t*)) {
    spill_arr_ptr_t sarr;
    spill_arr_ptr_init(&sarr);

    while (!(parser_peek_match(p, until_tkn) || parser_eof(p)) // while !eof (edge-case handling)
    ) {
        spill_arr_ptr_push(&sarr, call(p));
        parser_match_token(p, divider);
    }

    return parser_freeze_expr_spill_arr(p, &sarr);
}

ast_expr_t* parser_alloc_expr(parser_t* p) { return arena_alloc(p->arena, sizeof(ast_expr_t)); }

static ast_expr_t* parse_primary_expr_impl(parser_t* p, ast_expr_t* opt_atom) {
    token_t* first_tkn = parser_peek(p);
    token_type_e first_type = first_tkn->type;
    ast_expr_t* lhs = opt_atom;
    // if no atoms, try to parse atoms as lhs ~~~~~~~~~~~~~~~~~~~
    if (!lhs) {
        if (token_is_builtin_type_or_id(first_type)) {
            lhs = parse_id(p);
            // try struct init in form Foo{.thing = 1, .bar = 2}
            if (parser_peek_match(p, TOK_LBRACE)) {
                lhs = parse_expr_struct_init(p, lhs);
            }
        } else if (token_is_literal(first_type)) {
            lhs = parse_literal(p);
        } else if (first_type == TOK_LPAREN) {
            lhs = parse_grouping(p);
        }
    }
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // if atom, try to parse postunary, since postunary prec > preunary prec
    if (lhs && is_postunary_op(parser_peek(p)->type)) {
        return parse_postunary(p, lhs);
    }
    // try fn call too
    if (lhs && (parser_peek(p)->type == TOK_LPAREN || parser_peek(p)->type == TOK_GENERIC_SEP)) {
        return parse_fn_call(p, lhs);
    }
    // try subscript
    if (lhs && parser_peek(p)->type == TOK_LBRACK) {
        return parse_subscript(p, lhs);
    }
    // try ++x, etc.
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

ast_expr_t* parse_primary_expr(parser_t* p) { return parse_primary_expr_impl(p, NULL); }

ast_expr_t* parse_expr(parser_t* p) {
    ast_expr_t* lhs = parse_primary_expr_impl(p, NULL);
    return parse_expr_prec(p, lhs, PREC_INIT);
}

ast_expr_t* parse_expr_prec(parser_t* p, ast_expr_t* lhs, uint8_t prec) {
    token_type_e op = parser_peek(p)->type;
    if (!lhs && is_preunary_op(op)) {
        return parse_preunary_expr(p);
    }
    if (is_legal_binary_op(p, parser_peek(p)->type)) {
        return parse_binary(p, lhs, prec);
    }
    if (!lhs) {
        return parse_expr(p);
    }
    assert(lhs != NULL && "[parse_expr.c|parse_expr_prec] lhs is NULL");
    return lhs;
}

ast_expr_t* parse_preunary_expr(parser_t* p) {
    // special preunary case, &mut <expr> or &<expr>
    if (parser_peek(p)->type == TOK_AMPER) {
        return parse_expr_borrow(p);
    }
    token_t* op = parser_eat(p); // already been checked that this token is legit
    ast_expr_t* preunary_expr = parser_alloc_expr(p);
    // set op
    preunary_expr->type = AST_EXPR_PRE_UNARY;
    preunary_expr->first = op;
    preunary_expr->expr.unary.op = op;
    // get and set sub expression
    ast_expr_t* sub_expr = NULL;
    // things like sizeof(...)
    if (token_is_preunary_op_expecting_type(op->type)) {
        // sizeof(
        //       ^
        if (!parser_expect_token(p, TOK_LPAREN)) {
            return parser_sync_expr(p);
        }
        sub_expr = parse_expr_type(p);
        // sizeof(sub_expr)
        //                ^
        if (!parser_expect_token(p, TOK_RPAREN)) {
            return parser_sync_expr(p);
        }
    }
    // all others like ++x, --x
    else {
        sub_expr = parse_expr_prec(p, NULL, prec_preunary(op->type));
    }
    preunary_expr->expr.unary.expr = sub_expr;
    preunary_expr->last = parser_prev(p);
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
    token_ptr_slice_t id_slice = parse_id_token_slice(p, TOK_SCOPE_RES);
    // handle failure edge case, this is usually safe if we knew to enter this function
    if (id_slice.len == 0) {
        return parser_sync_expr(p);
    }
    id_expr->expr.id.slice = id_slice;
    id_expr->type = AST_EXPR_ID;
    id_expr->first = id_expr->expr.id.slice.start[0];
    id_expr->last = id_expr->expr.id.slice.start[id_expr->expr.id.slice.len - 1];
    return id_expr;
}

ast_expr_t* parse_expr_from_id_slice(parser_t* p, token_ptr_slice_t id_slice) {
    // consider this, but parser may be overstepping boundaries here
    // if (token_is_builtin_type(id_slice.start[0]->type) && id_slice.len > 0) {
    //    compiler_error_list_emplace(p->error_list, id_slice.start[0], ERR_EXPECTED_EXPRESSION);
    //    return parser_sync_expr(p);
    //}
    ast_expr_t* id_expr = parser_alloc_expr(p);
    id_expr->expr.id.slice = id_slice;
    id_expr->type = AST_EXPR_ID;
    id_expr->first = id_expr->expr.id.slice.start[0];
    id_expr->last = id_expr->expr.id.slice.start[id_expr->expr.id.slice.len - 1];
    ast_expr_t* lhs = parse_primary_expr_impl(p, id_expr);
    return parse_expr_prec(p, lhs, PREC_INIT);
}

token_t* parse_var_name(parser_t* p) {
    return parser_expect_token_with_err_code(p, TOK_IDENTIFIER, ERR_EXPECTED_VARIABLE_NAME);
}

static bool binary_bind_right(token_type_e curr_op, token_type_e next_op) {
    uint8_t curr_prec = prec_binary(curr_op);
    uint8_t next_prec = prec_binary(next_op);
    return (next_prec < curr_prec) ||
           (next_prec == curr_prec && is_right_assoc_from_prec(curr_prec));
}

ast_expr_t* parse_binary(parser_t* p, ast_expr_t* lhs, uint8_t max_prec) {
    ast_expr_t* binary_expr = parser_alloc_expr(p);
    token_t* op_tkn = parser_eat(p); // already verfied legit
    binary_expr->type = AST_EXPR_BINARY;
    binary_expr->expr.binary.lhs = lhs;
    binary_expr->expr.binary.op = op_tkn;
    max_prec = (max_prec >= prec_binary(op_tkn->type)) ? max_prec : prec_binary(op_tkn->type);
    ast_expr_t* middle_expr = NULL;
    // handle special binary ops
    if (op_tkn->type == TOK_AS) {
        middle_expr = parse_expr_type(p);
    } else if (op_tkn->type == TOK_IS) {
        middle_expr = parse_expr_variant_decomp(p);
    } else {
        middle_expr = parse_primary_expr_impl(p, NULL);
    }
    token_type_e curr_op = op_tkn->type;
    token_type_e next_op = parser_peek(p)->type;

    while (binary_bind_right(curr_op, next_op)) {
        middle_expr = parse_expr_prec(p, middle_expr, prec_binary(next_op));
        curr_op = next_op, next_op = parser_peek(p)->type;
    }
    binary_expr->expr.binary.rhs = middle_expr;
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
    if (parser_peek_match(p, TOK_GENERIC_SEP)) {
        call_expr->expr.fn_call.is_generic = true;
        parser_mode_e saved = parser_mode(p);
        parser_mode_set(p, PARSER_MODE_BAN_LT_GT);
        call_expr->expr.fn_call.generic_args = parse_slice_of_generic_args(p);
        parser_mode_set(p, saved);
    } else {
        call_expr->expr.fn_call.is_generic = false;
    }
    token_t* lparen = parser_expect_token(p, TOK_LPAREN); // already verfied legit
    call_expr->type = AST_EXPR_FN_CALL;
    call_expr->expr.fn_call.left_expr = lhs;
    call_expr->expr.fn_call.left_paren = lparen;

    spill_arr_ptr_t args;
    spill_arr_ptr_init(&args);

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

ast_expr_t* parse_expr_type(parser_t* p) {
    ast_expr_t* s = parser_alloc_expr(p);
    s->type = AST_EXPR_TYPE;
    ast_type_t* type = parse_type(p);
    if (type->tag == AST_TYPE_INVALID) {
        return parser_sync_expr(p);
    }
    s->expr.type_expr.type = type;
    s->first = type->first;
    s->last = type->last;
    return s;
}

ast_expr_t* parse_expr_struct_member_init(parser_t* p) {
    ast_expr_t* s = parser_alloc_expr(p);
    s->type = AST_EXPR_STRUCT_MEMBER_INIT;
    if (!parser_expect_token(p, TOK_DOT)) {
        return parser_sync_expr(p);
    }
    token_t* name = parser_match_token(p, TOK_IDENTIFIER);
    if (!name) {
        compiler_error_list_emplace(p->error_list, name, ERR_EXPECTED_IDENTIFER);
        return parser_sync_expr(p);
    }
    token_t* assign_op = parser_match_token_call(p, &token_is_assignment_init);
    if (!assign_op) {
        compiler_error_list_emplace(p->error_list, assign_op, ERR_EXPECTED_ASSIGNMENT);
        return parser_sync_expr(p);
    }
    s->expr.struct_member_init.id = name;
    s->expr.struct_member_init.assign_op = assign_op;
    s->expr.struct_member_init.value = parse_expr(p);
    // now we should be returning something in the form .foo = some_expr
    return s;
}

ast_expr_t* parse_expr_struct_init(parser_t* p, ast_expr_t* opt_id_lhs) {
    ast_expr_t* s = parser_alloc_expr(p);
    s->type = AST_EXPR_STRUCT_INIT;
    // if the optional id lhs is NULL, parse an id
    if (!opt_id_lhs) {
        opt_id_lhs = parse_id(p);
    }
    // verfify it's 100% an id (matters if passed in)
    if (opt_id_lhs->type != AST_EXPR_ID) {
        compiler_error_list_emplace(p->error_list, parser_prev(p), ERR_EXPECTED_IDENTIFER);
        return parser_sync_expr(p);
    }
    // extract the id slice from the id, this should be safe given the above check
    token_ptr_slice_t id = opt_id_lhs->expr.id.slice;
    s->expr.struct_init.id = id;
    parser_expect_token(p, TOK_LBRACE);
    s->expr.struct_init.member_inits =
        parse_slice_of_exprs_call(p, TOK_COMMA, TOK_RBRACE, &parse_expr_struct_member_init);
    parser_expect_token(p, TOK_RBRACE);
    s->first = id.start[0]; // safe becuz the id should be valid given the check passed
    s->last = parser_prev(p);
    return s;
}

ast_expr_t* parse_expr_borrow(parser_t* p) {
    ast_expr_t* s = parser_alloc_expr(p);
    s->type = AST_EXPR_BORROW;
    token_t* amper = parser_expect_token(p, TOK_AMPER);
    if (!amper) {
        return parser_sync_expr(p);
    }
    s->expr.borrow.mut = parser_match_token(p, TOK_MUT);
    s->expr.borrow.borrowed = parse_expr(p);
    s->first = amper;
    s->last = parser_prev(p);
    return s;
}

ast_expr_t* parse_expr_variant_decomp(parser_t* p) {
    ast_expr_t* s = parser_alloc_expr(p);
    s->type = AST_EXPR_VARIANT_DECOMP;
    token_t* first = parser_peek(p);
    s->expr.variant_decomp.id = parse_id_token_slice(p, TOK_SCOPE_RES);
    if (parser_match_token(p, TOK_LPAREN)) {
        s->expr.variant_decomp.vars = parse_slice_of_params(p, TOK_COMMA, TOK_RPAREN);
        parser_expect_token(p, TOK_RPAREN);
    } else {
        ast_slice_of_params_t vars = {.start = NULL, .len = 0};
        s->expr.variant_decomp.vars = vars;
    }
    s->first = first;
    s->last = parser_prev(p);
    return s;
}
