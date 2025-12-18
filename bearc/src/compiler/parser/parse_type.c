//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/parser/parse_type.h"
#include "compiler/parser/parser.h"
#include "compiler/token.h"
#include <assert.h>

ast_type_t* parser_alloc_type(parser_t* p) { return arena_alloc(p->arena, sizeof(ast_type_t)); }

ast_type_t* parser_sync_type(parser_t* p) {
    token_range_t range = parser_sync(p);
    ast_type_t* dummy_type = parser_alloc_type(p);
    dummy_type->tag = AST_TYPE_INVALID;
    dummy_type->first = range.first;
    dummy_type->last = range.last;
    return dummy_type;
}

ast_type_t* parse_type_base(parser_t* p, bool pre_mut) {
    ast_type_t* base = parser_alloc_type(p);
    // handle pre-mut
    base->type.base.mut = pre_mut;
    token_t* mut_tkn = NULL;
    if (pre_mut) {
        mut_tkn = parser_eat(p);
    }
    base->type.base.id = parse_token_ptr_slice(p, TOK_SCOPE_RES);
    // handle post-mut
    bool post_mut = parser_match_token(p, TOK_MUT); // optionally consume post-mut as bool
    base->type.base.mut = post_mut;
    if (post_mut && !pre_mut) {
        base->first = base->type.base.id.start[0];
    } else {
        assert(mut_tkn != NULL && "[parse_type_base] mut_tkn must not be NULL");
        base->first = mut_tkn;
    }
    base->last = base->type.base.id.start[base->type.base.id.len - 1];
    return base;
}

ast_type_t* parse_type(parser_t* p) {
    token_t* first_tkn = parser_peek(p);
    token_type_e first_type = first_tkn->type;
    ast_type_t* base = NULL;
    if (token_is_builtin_type_or_id(first_type)) {
        base = parse_type_base(p, false); // pre-mut = false
    } else if (first_type == TOK_MUT) {
        base = parse_type_base(p, true); // pre-mut = true
    }
    // TODO parse refs & qualifiers from base
    compiler_error_list_emplace(p->error_list, first_tkn, ERR_EXPECTED_TYPE);
    return parser_sync_type(p);
}
