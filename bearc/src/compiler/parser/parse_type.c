//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/parser/parse_type.h"
#include "compiler/ast/expr.h"
#include "compiler/diagnostics/error_codes.h"
#include "compiler/diagnostics/error_list.h"
#include "compiler/parser/parse_expr.h"
#include "compiler/parser/parser.h"
#include "compiler/parser/token_eaters.h"
#include "compiler/token.h"
#include "utils/arena.h"
#include "utils/spill_arr.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

ast_type_t* parser_alloc_type(parser_t* p) { return arena_alloc(p->arena, sizeof(ast_type_t)); }

ast_type_t* parser_sync_type(parser_t* p) {
    token_range_t range = parser_sync(p);
    ast_type_t* dummy_type = parser_alloc_type(p);
    dummy_type->tag = AST_TYPE_INVALID;
    dummy_type->first = range.first;
    dummy_type->last = range.last;
    return dummy_type;
}

ast_slice_of_params_t parser_freeze_params_spill_arr(parser_t* p, spill_arr_ptr_t* sarr) {
    ast_slice_of_params_t slice = {
        .start = (ast_param_t**)arena_alloc(p->arena, sarr->size * sizeof(ast_param_t*)),
        .len = sarr->size,
    };
    spill_arr_ptr_flat_copy((void**)slice.start, sarr);
    spill_arr_ptr_destroy(sarr);
    return slice;
}

ast_slice_of_params_t parse_slice_of_params(parser_t* p, token_type_e divider,
                                            token_type_e terminator) {
    spill_arr_ptr_t sarr;
    spill_arr_ptr_init(&sarr);

    while (!parser_peek_match(p, terminator) && !parser_eof(p)) {
        spill_arr_ptr_push(&sarr, parse_param(p));
        if (!parser_peek_match(p, terminator)) {
            parser_expect_token(p, divider);
        }
    }

    return parser_freeze_params_spill_arr(p, &sarr);
}

static ast_type_t* parse_type_base_impl(parser_t* p, bool pre_mut, token_ptr_slice_t id_slice) {
    ast_type_t* base = parser_alloc_type(p);
    // self-referential for canonical_base, although this shouldn't be an issue
    base->canonical_base = base;
    // handle pre-mut
    token_t* pre_mut_tkn = NULL;
    if (pre_mut) {
        pre_mut_tkn = parser_eat(p);
        // handle redundant leading muts
        while (parser_match_token(p, TOK_MUT)) {
            compiler_error_list_emplace(p->error_list, parser_prev(p), ERR_REDUNDANT_MUT);
        }
        // parse base type
        base->type.base.id = parse_id_token_slice(p, TOK_SCOPE_RES);
    } else {
        // base id must already be parsed and passed in
        base->type.base.id = id_slice;
    }
    // handle post-mut
    bool post_mut = parser_match_token(p, TOK_MUT); // optionally consume post-mut as bool

    base->type.base.mut = pre_mut || post_mut;

    // handle tkn ordering due to mut order
    if (post_mut && !pre_mut) {
        base->first = base->type.base.id.start[0];
        base->last = parser_prev(p); // gets post mut token
    } else if (post_mut && pre_mut) {
        assert(pre_mut_tkn != NULL && "[parse_type_base] mut_tkn must not be NULL");
        base->first = pre_mut_tkn;
        base->last = parser_prev(p); // gets post mut token
        compiler_error_list_emplace(p->error_list, parser_prev(p), ERR_REDUNDANT_MUT);
    } else if (!post_mut && !pre_mut) {
        base->first = base->type.base.id.start[0];
        base->last = base->type.base.id.start[base->type.base.id.len - 1];
    } else {
        base->first = pre_mut_tkn;
        base->last = base->type.base.id.start[base->type.base.id.len - 1];
    }
    base->tag = AST_TYPE_BASE;
    return base;
}

ast_type_t* parse_type_base_with_leading_id(parser_t* p, token_ptr_slice_t id_slice) {
    return parse_type_base_impl(p, false, id_slice);
}

ast_type_t* parse_type_base_with_leading_mut(parser_t* p) {
    token_ptr_slice_t dummy = {.start = NULL, .len = 0};
    return parse_type_base_impl(p, true, dummy);
}

ast_type_t* parse_type_ref(parser_t* p, ast_type_t* inner);

static ast_type_t* parse_type_impl(parser_t* p, token_ptr_slice_t leading_id, bool has_leading_id) {
    token_t* first_tkn = parser_peek(p);
    token_type_e first_type = first_tkn->type;

    ast_type_t* inner = NULL;

    // parse base types
    if (has_leading_id) {
        inner = parse_type_base_with_leading_id(p, leading_id); // pre-mut = false
    } else if (first_type == TOK_MUT) {
        inner = parse_type_base_with_leading_mut(p);
    } else {
        leading_id = parse_id_token_slice(p, TOK_SCOPE_RES);
        inner = parse_type_base_with_leading_id(p, leading_id);
    }

    if (token_is_generic_opener(parser_peek(p)->type)) {
        inner = parse_type_generic(p, inner);
    }

    // parse type modifiers
    while (token_is_ref_or_ptr(parser_peek(p)->type)) {
        inner = parse_type_ref(p, inner);
    }
    while (parser_peek(p)->type == TOK_LBRACK && parser_peek_n(p, 1)->type == TOK_AMPER) {
        inner = parse_type_slice(p, inner);
    }
    while (parser_peek(p)->type == TOK_LBRACK) {
        inner = parse_type_arr(p, inner);
    }
    if (inner) {
        return inner;
    }
    compiler_error_list_emplace(p->error_list, first_tkn, ERR_EXPECTED_TYPE);
    return parser_sync_type(p);
}

ast_type_t* parse_type_with_leading_id(parser_t* p, token_ptr_slice_t id_slice) {
    return parse_type_impl(p, id_slice, true);
}

ast_type_t* parse_type(parser_t* p) {
    token_ptr_slice_t dummy = {.start = NULL, .len = 0};
    return parse_type_impl(p, dummy, false); //! has_leading_id
}

ast_type_t* parse_type_ref(parser_t* p, ast_type_t* inner) {
    ast_type_t* outer = parser_alloc_type(p);
    outer->type.ref.modifier = parser_eat(p); // definitely fine because we know to be in this func
    outer->type.ref.mut = parser_match_token(p, TOK_MUT); // match into bool
    outer->canonical_base = inner->canonical_base;
    outer->type.ref.inner = inner;
    outer->tag = AST_TYPE_REF_PTR;
    outer->first = inner->first;
    outer->last = parser_prev(p);
    return outer;
}

ast_type_t* parse_type_arr(parser_t* p, ast_type_t* inner) {
    ast_type_t* outer = parser_alloc_type(p);
    outer->tag = AST_TYPE_ARR;

    // handle canonical base
    outer->canonical_base = inner->canonical_base;

    // handle [size_expr]
    parser_expect_token(p, TOK_LBRACK); // should be fine because we know to be in this func
    ast_expr_t* size_expr = parse_expr(p);
    if (size_expr->type == AST_EXPR_INVALID) {
        return parser_sync_type(p);
    }
    outer->type.arr.size_expr = size_expr;
    token_t* rbrack = parser_expect_token(p, TOK_RBRACK);
    if (!rbrack) {
        return parser_sync_type(p);
    }
    outer->type.arr.mut = parser_match_token(p, TOK_MUT); // match into bool
    outer->type.arr.inner = inner;
    outer->first = inner->first;
    outer->last = parser_prev(p);
    return outer;
}

ast_type_t* parse_type_slice(parser_t* p, ast_type_t* inner) {
    ast_type_t* outer = parser_alloc_type(p);
    outer->tag = AST_TYPE_SLICE;
    // handle canonical base
    outer->canonical_base = inner->canonical_base;
    // handle [&]
    // should be fine because we know to be in this func
    if (!parser_expect_token(p, TOK_LBRACK)) {
        return parser_sync_type(p);
    }
    if (!parser_expect_token(p, TOK_AMPER)) {
        return parser_sync_type(p);
    }
    outer->type.slice.mut = parser_match_token(p, TOK_MUT); // match into bool
    if (!parser_expect_token(p, TOK_RBRACK)) {
        return parser_sync_type(p);
    }
    outer->type.slice.inner = inner;
    outer->first = inner->first;
    outer->last = parser_prev(p);
    return outer;
}

ast_generic_arg_t* parse_generic_arg(parser_t* p) {
    ast_generic_arg_t* arg = arena_alloc(p->arena, sizeof(ast_generic_arg_t));
    token_type_e next_type = parser_peek(p)->type;
    ast_expr_t* expr = NULL;
    ast_type_t* type = NULL;

    bool is_type = false;
    // parse as type or id ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (token_is_builtin_type_or_id(parser_peek(p)->type)) {
        token_ptr_slice_t leading_id = parse_id_token_slice(p, TOK_SCOPE_RES);
        next_type = parser_peek(p)->type;
        if (token_is_posttype_indicator(next_type)) {
            is_type = true;
            type = parse_type_with_leading_id(p, leading_id);
        } else {
            is_type = false;
            expr = parse_expr_from_id_slice(p, leading_id);
        }
    } else {
        // parse things that have a leading expr
        is_type = false;
        expr = parse_expr(p);
    }
    if (token_is_pretype_idicator(parser_peek(p)->type)) {
        is_type = true;
        type = parse_type(p);
    }
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    // set according to is_type
    arg->valid = true;
    if (is_type) {
        if (type->tag == AST_TYPE_INVALID) {
            arg->valid = false;
        }
        arg->tag = AST_GENERIC_ARG_TYPE;
        assert(type != NULL && "[parse_generic_arg] arg type must not be NULL");
        arg->arg.type = type;
    } else {
        // handle invalid leading expr
        if (expr->type == AST_EXPR_INVALID) {
            arg->valid = false;
        }
        arg->tag = AST_GENERIC_ARG_EXPR;
        assert(expr != NULL && "[parse_generic_arg] arg expr must not be NULL");
        arg->arg.expr = expr;
    }
    return arg;
}

ast_generic_arg_slice_t parser_freeze_generic_arg_spill_arr(parser_t* p, spill_arr_ptr_t* sarr) {
    ast_generic_arg_slice_t slice = {
        .start =
            (ast_generic_arg_t**)arena_alloc(p->arena, sarr->size * sizeof(ast_generic_arg_t*)),
        .len = sarr->size,
    };
    spill_arr_ptr_flat_copy((void**)slice.start, sarr);
    spill_arr_ptr_destroy(sarr);
    return slice;
}

ast_generic_arg_slice_t parse_generic_arg_slice(parser_t* p) {

    spill_arr_ptr_t sarr;
    spill_arr_ptr_init(&sarr);

    token_t* opener =
        parser_expect_token_call(p, &token_is_generic_opener, ERR_EXPECT_GENERIC_OPENER);
    // when just, ::, just expect one thing, like box::thing
    ast_generic_arg_t* arg = NULL;
    bool valid = false;
    if (opener->type == TOK_TYPE_MOD && !parser_match_token(p, TOK_LT)) {
        arg = parse_generic_arg(p);
        valid |= arg->valid;
        spill_arr_ptr_push(&sarr, arg);
    }
    // otherwise expect foo::<garg1, garg2> or foo<garg1, garg2>
    else {
        while (!parser_peek_match(p, TOK_GT) && !parser_eof(p)) {
            arg = parse_generic_arg(p);
            valid |= arg->valid;
            spill_arr_ptr_push(&sarr, arg);
            if (!parser_peek_match(p, TOK_GT) && !parser_peek_match(p, TOK_TYPE_MOD)) {
                parser_expect_token(p, TOK_COMMA);
            }
        }
        token_t* gt = parser_expect_token(p, TOK_GT);
        valid = gt; // false if gt == NULL
    }
    ast_generic_arg_slice_t args = parser_freeze_generic_arg_spill_arr(p, &sarr);
    args.valid = valid;
    return args;
}

ast_type_t* parse_type_generic(parser_t* p, ast_type_t* inner) {
    ast_type_t* outer = parser_alloc_type(p);
    outer->tag = AST_TYPE_GENERIC;

    // handle canonical base
    if (!(inner->tag == AST_TYPE_BASE)) {
        compiler_error_list_emplace(p->error_list, inner->first, ERR_EXPECTED_BASE_TYPE_IN_GENERIC);
        return parser_sync_type(p);
    }
    outer->type.generic.inner = inner;
    outer->canonical_base = inner->canonical_base;
    parser_disable_bool_comparision(p); // cleaner template parsing from < and > issues
    ast_generic_arg_slice_t args = parse_generic_arg_slice(p);
    parser_enable_bool_comparision(p); // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    if (!args.valid) {
        return parser_sync_type(p);
    }
    outer->type.generic.generic_args = args;
    outer->first = inner->first;
    outer->last = parser_prev(p);
    return outer;
}
