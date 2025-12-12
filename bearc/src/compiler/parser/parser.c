// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "compiler/parser/parser.h"
#include "compiler/diagnostics/error_codes.h"
#include "compiler/diagnostics/error_list.h"
#include "compiler/token.h"
#include "utils/vector.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/**
 * primary parser structure
 * tracks a position ptr along a vector of token_t
 */
typedef struct {
    vector_t tokens;
    size_t pos;
} parser_t;

// ----------- token consumption primitive functions -------

// consume a token
token_t* parser_eat(parser_t* parser) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    if (tkn->type != TOK_EOF) {
        parser->pos++;
    }
    return tkn;
}

// peek current uneaten token without consuming it
token_t* parser_peek(parser_t* parser) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    return tkn;
}

token_t* parser_peek_n(parser_t* parser, size_t n) {
    size_t idx = parser->pos + n;
    if (idx >= parser->tokens.size) {
        return parser_peek(parser); // return EOF
    }
    return vector_at(&parser->tokens, idx);
}

// see last eaten token
token_t* parser_prev(parser_t* parser) {
    if (parser->pos == 0) {
        return NULL;
    }
    token_t* tkn = vector_at(&parser->tokens, parser->pos - 1);
    return tkn;
}

/*
 * peek then conditionally eat
 * \return token_t* to consumed token or NULL if not matched
 */
token_t* parser_match_token(parser_t* parser, token_type_e type) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    if (tkn->type == type) {
        if (tkn->type != TOK_EOF) {
            parser->pos++;
        }
        return tkn;
    }
    return NULL;
}

//------------- match forward decls --------------
bool parser_match_is_builtin_type(token_type_e t);

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

/*
 * peek then conditionally eat based on a match function call, which takes a token_type_e and
 * returns bool if valid, else false
 * \return token_t* to consumed token or NULL if not matched
 */
token_t* parser_match_token_call(parser_t* parser, bool (*match)(token_type_e)) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    if (match(tkn->type)) {
        if (tkn->type != TOK_EOF) {
            parser->pos++;
        }
        return tkn;
    }
    return NULL;
}

// eat if current token matches specified type or return NULL and add to error_list
token_t* parser_expect_token(parser_t* parser, token_type_e expected_type,
                             compiler_error_list_t* error_list) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    if (tkn->type == expected_type) {
        parser->pos++;
        return tkn;
    }
    compiler_error_list_emplace_expected_token(error_list, tkn, ERR_EXPECTED_TOKEN, expected_type);
    return NULL;
}

// eat or return NULL and add a specific error to error_list (not just Expected token: __)
// matches based on a specified token_type_e
token_t* parser_expect_token_with_err_code(parser_t* parser, token_type_e expected_type,
                                           compiler_error_list_t* error_list, error_code_e code) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    if (tkn->type == expected_type) {
        parser->pos++;
        return tkn;
    }
    compiler_error_list_emplace(error_list, tkn, code);
    return NULL;
}

// eat or return NULL and add a specific error to error_list (not just Expected token: __)
// uses a match call that returns bool based on a token_type_e
token_t* parser_expect_token_call(parser_t* parser, bool (*match)(token_type_e),
                                  compiler_error_list_t* error_list, error_code_e code) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    if (match(tkn->type)) {
        parser->pos++;
        return tkn;
    }
    compiler_error_list_emplace(error_list, tkn, code);
    return NULL;
}

// returns true when parser is at EOF
bool parser_eof(parser_t* parser) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    return tkn->type == TOK_EOF;
}

// map containing look-ups for builtin types
static const bool parser_builtin_type_map[TOK__NUM] = {
    [TOK_CHAR] = true,     [TOK_U8] = true,  [TOK_I8] = true,   [TOK_I32] = true,
    [TOK_U32] = true,      [TOK_I64] = true, [TOK_U64] = true,  [TOK_USIZE] = true,
    [TOK_F32] = true,      [TOK_F64] = true, [TOK_BOOL] = true, [TOK_STR] = true,
    [TOK_MODULE] = true,   [TOK_FN] = true,  [TOK_VAR] = true,  [TOK_VOID] = true,
    [TOK_SELF_TYPE] = true};

static const bool parser_builtin_type_mod_map[TOK__NUM] = {
    [TOK_BOX] = true, [TOK_BAG] = true, [TOK_OPT] = true, [TOK_RES] = true};

// match helpers
bool parser_match_is_builtin_type(token_type_e t) { return parser_builtin_type_map[t]; }
bool parser_match_is_builtin_type_or_id(token_type_e t) {
    return parser_builtin_type_map[t] || t == TOK_IDENTIFIER;
}
bool parser_match_is_builtin_type_mod(token_type_e t) { return parser_builtin_type_mod_map[t]; }

// ^^^^^^^^^^^^^^^^ token consumption primitive functions ^^^^^^^^^^^^^^^^^^^^^^^^^^^

// primary function for parsing a file into an ast
ast_t parser_build_ast_from_file(const char* file_name, vector_t token_vec,
                                 compiler_error_list_t* error_list) {
    // position tracking, consuming tokens, etc
    parser_t parser = {.tokens = token_vec, .pos = 0};

    // init ast & node arena
    ast_t ast = ast_create(file_name);

    // TODO, AST building up logic here
    // right now this is just some placeholder logic to test the basic parser functions
    // no ast is being raised, we're just running through the tokens
    token_t* tkn = NULL; // scratch token
    while (!parser_eof(&parser)) {
        tkn = parser_match_token(&parser, TOK_INDETERMINATE);
        if (tkn) {
            compiler_error_list_emplace(error_list, tkn, ERR_ILLEGAL_IDENTIFER);
        }
        // expect id or other after builtin type
        else if (parser_match_token_call(&parser, &parser_match_is_builtin_type)) {
            tkn = parser_match_token(&parser, TOK_IDENTIFIER);
            if (!tkn) {
                tkn = parser_match_token(&parser, TOK_SELF_ID);
            }
            if (!tkn) {
                tkn = parser_match_token(&parser, TOK_MUT);
            }
            if (!tkn) {
                tkn = parser_match_token(&parser, TOK_STAR);
            }
            if (!tkn) {
                tkn = parser_match_token(&parser, TOK_AMPER);
            }
            if (!tkn) {
                tkn = parser_match_token(&parser, TOK_LBRACK);
            }

            if (!tkn) {
                compiler_error_list_emplace(error_list, tkn = parser_eat(&parser),
                                            ERR_EXPECTED_IDENTIFIER);
            }

        }

        // expect type after rarrow
        else if (parser_match_token(&parser, TOK_RARROW)) {
            parser_expect_token_call(&parser, &parser_match_is_builtin_type_or_id, error_list,
                                     ERR_EXPECTED_TYPE);
        }

        // just consume
        else {
            parser_eat(&parser);
        }
    }

    return ast;
}
