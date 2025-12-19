//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/diagnostics/error_codes.h"
#include "compiler/diagnostics/error_list.h"
#include "compiler/parser/parser.h"
#include "compiler/token.h"
#include "utils/vector.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// ----------- token consumption primitive functions -------

// consume a token
token_t* parser_eat(parser_t* parser) {
    token_t* tkn = vector_at(parser->tokens, parser->pos);
    if (tkn->type != TOK_EOF) {
        parser->pos++;
    }
    return tkn;
}

// peek current uneaten token without consuming it
token_t* parser_peek(parser_t* parser) {
    token_t* tkn = vector_at(parser->tokens, parser->pos);
    return tkn;
}

token_t* parser_peek_n(parser_t* parser, size_t n) {
    size_t idx = parser->pos + n;
    if (idx >= parser->tokens->size) {
        return parser_peek(parser); // return EOF
    }
    return vector_at(parser->tokens, idx);
}

// see last eaten token
token_t* parser_prev(parser_t* parser) {
    if (parser->pos == 0) {
        return NULL;
    }
    token_t* tkn = vector_at(parser->tokens, parser->pos - 1);
    return tkn;
}

/*
 * peek then conditionally eat
 * \return token_t* to consumed token or NULL if not matched
 */
token_t* parser_match_token(parser_t* parser, token_type_e type) {
    token_t* tkn = vector_at(parser->tokens, parser->pos);
    if (tkn->type == type) {
        if (tkn->type != TOK_EOF) {
            parser->pos++;
        }
        return tkn;
    }
    return NULL;
}

//------------- match forward decls --------------
bool token_is_builtin_type(token_type_e t);

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

/*
 * peek then conditionally eat based on a match function call, which takes a token_type_e and
 * returns bool if valid, else false
 * \return token_t* to consumed token or NULL if not matched
 */
token_t* parser_match_token_call(parser_t* parser, bool (*match)(token_type_e)) {
    token_t* tkn = vector_at(parser->tokens, parser->pos);
    if (match(tkn->type)) {
        if (tkn->type != TOK_EOF) {
            parser->pos++;
        }
        return tkn;
    }
    return NULL;
}

// eat if current token matches specified type or return NULL and add to error_list
token_t* parser_expect_token(parser_t* parser, token_type_e expected_type) {
    token_t* tkn = vector_at(parser->tokens, parser->pos);
    if (tkn->type == expected_type) {
        parser->pos++;
        return tkn;
    }
    compiler_error_list_emplace_expected_token(parser->error_list, tkn, ERR_EXPECTED_TOKEN,
                                               expected_type);
    return NULL;
}

// eat or return NULL and add a specific error to error_list (not just Expected token: __)
// matches based on a specified token_type_e
token_t* parser_expect_token_with_err_code(parser_t* parser, token_type_e expected_type,
                                           error_code_e code) {
    token_t* tkn = vector_at(parser->tokens, parser->pos);
    if (tkn->type == expected_type) {
        parser->pos++;
        return tkn;
    }
    compiler_error_list_emplace(parser->error_list, tkn, code);
    return NULL;
}

// eat or return NULL and add a specific error to error_list (not just Expected token: __)
// uses a match call that returns bool based on a token_type_e
token_t* parser_expect_token_call(parser_t* parser, bool (*match)(token_type_e),
                                  error_code_e code) {
    token_t* tkn = vector_at(parser->tokens, parser->pos);
    if (match(tkn->type)) {
        parser->pos++;
        return tkn;
    }
    compiler_error_list_emplace(parser->error_list, tkn, code);
    return NULL;
}

// returns true when parser is at EOF
bool parser_eof(const parser_t* parser) {
    token_t* tkn = vector_at(parser->tokens, parser->pos);
    return tkn->type == TOK_EOF;
}

bool token_is_syncable_delim(token_type_e t);

token_range_t parser_sync(parser_t* p) {
    token_t* first_tkn = parser_peek(p);
    token_t* last_tkn = first_tkn; // init in case loop never runs!
    while (!parser_eof(p)) {
        token_t* curr = parser_peek(p);
        if (token_is_syncable_delim(curr->type)) {
            break;
        }
        last_tkn = parser_eat(p);
    }
    token_range_t range = {
        .first = first_tkn,
        .last = last_tkn,
    };
    return range;
}

// map containing look-ups for builtin types
static const bool parser_builtin_type_map[TOK__NUM] = {
    [TOK_CHAR] = true,     [TOK_U8] = true,    [TOK_I8] = true,  [TOK_I16] = true,
    [TOK_U16] = true,      [TOK_I32] = true,   [TOK_U32] = true, [TOK_I64] = true,
    [TOK_U64] = true,      [TOK_USIZE] = true, [TOK_F32] = true, [TOK_F64] = true,
    [TOK_BOOL] = true,     [TOK_STR] = true,   [TOK_VAR] = true, [TOK_VOID] = true,
    [TOK_SELF_TYPE] = true};

static const bool parser_builtin_type_mod_map[TOK__NUM] = {
    [TOK_BOX] = true, [TOK_BAG] = true, [TOK_OPT] = true, [TOK_RES] = true};

// match helpers
bool token_is_builtin_type(token_type_e t) { return parser_builtin_type_map[t]; }
bool token_is_builtin_type_or_id(token_type_e t) {
    return parser_builtin_type_map[t] || t == TOK_IDENTIFIER || parser_builtin_type_mod_map[t];
}
bool token_is_builtin_type_mod(token_type_e t) { return parser_builtin_type_mod_map[t]; }

bool token_is_literal(token_type_e t) {
    return t == TOK_BOOL_LIT_TRUE || t == TOK_BOOL_LIT_TRUE || t == TOK_INT_LIT ||
           t == TOK_DOUB_LIT || t == TOK_STR_LIT || t == TOK_CHAR_LIT;
}

bool token_is_syncable_delim(token_type_e t) {
    return t == TOK_LBRACE || t == TOK_SEMICOLON || t == TOK_RBRACE || t == TOK_RPAREN ||
           t == TOK_RBRACK;
}

bool token_is_assignment_init(token_type_e t) { return t == TOK_ASSIGN_MOVE || t == TOK_ASSIGN_EQ; }

bool token_is_type_indicator(token_type_e t) {
    return t == TOK_IDENTIFIER || t == TOK_MUT || t == TOK_AMPER || t == TOK_STAR;
}

bool token_is_ref_or_ptr(token_type_e t) { return t == TOK_AMPER || t == TOK_STAR; }

bool token_is_function_leading_kw(token_type_e t) {
    return t == TOK_FN || t == TOK_MT || t == TOK_DT;
}

// ^^^^^^^^^^^^^^^^ token consumption primitive functions ^^^^^^^^^^^^^^^^^^^^^^^^^^^
