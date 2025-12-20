//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/parser/parse_type.h"
#include "compiler/ast/expr.h"
#include "compiler/parser/parse_expr.h"
#include "compiler/parser/parser.h"
#include "compiler/parser/token_eaters.h"
#include "compiler/token.h"
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
    spill_arr_ptr_t sarr = spill_arr_ptr_create();

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
    // handle pre-mut
    token_t* pre_mut_tkn = NULL;
    if (pre_mut) {
        pre_mut_tkn = parser_eat(p);
        base->type.base.id = parse_token_ptr_slice(p, TOK_SCOPE_RES);
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
        leading_id = parse_token_ptr_slice(p, TOK_SCOPE_RES);
        inner = parse_type_base_with_leading_id(p, leading_id);
    }

    // parse type modifiers
    while (token_is_ref_or_ptr(parser_peek(p)->type)) {
        inner = parse_type_ref(p, inner);
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
    if (inner->tag == AST_TYPE_BASE) {
        outer->type.ref.canonical_base = inner;
    } else {
        outer->type.ref.canonical_base = inner->type.ref.canonical_base;
    }
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
    if (inner->tag == AST_TYPE_BASE) {
        outer->type.arr.canonical_base = inner;
    } else {
        outer->type.arr.canonical_base = inner->type.ref.canonical_base;
    }
    // handle [size_expr]
    outer->type.arr.lbrack = parser_eat(p); // definitely fine because we know to be in this func
    ast_expr_t* size_expr = parse_expr(p);
    if (size_expr->type == AST_EXPR_INVALID) {
        return parser_sync_type(p);
    }
    outer->type.arr.size_expr = size_expr;
    token_t* rbrack = parser_expect_token(p, TOK_RBRACK);
    if (!rbrack) {
        return parser_sync_type(p);
    }
    outer->type.arr.rbrack = rbrack;
    outer->type.arr.mut = parser_match_token(p, TOK_MUT); // match into bool
    outer->type.arr.inner = inner;
    outer->first = inner->first;
    outer->last = parser_prev(p);
    return outer;
}
