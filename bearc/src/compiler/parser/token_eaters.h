//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_PARSER_TOKEN_EATERS_H
#define COMPILER_PARSER_TOKEN_EATERS_H
#include "compiler/diagnostics/error_codes.h"
#include "compiler/parser/parser.h"
#include "compiler/token.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
/// consume a token
token_t* parser_eat(parser_t* parser);

/// peek current uneaten token without consuming it
token_t* parser_peek(parser_t* parser);

token_t* parser_peek_n(parser_t* parser, size_t n);

/// see last eaten token
token_t* parser_prev(parser_t* parser);
/**
 * peek then conditionally eat
 * \return token_t* to consumed token or NULL if not matched
 */
token_t* parser_match_token(parser_t* parser, token_type_e type);

/**
 * peek then conditionally eat based on a match function call, which takes a token_type_e and
 * returns bool if valid, else false
 * \return token_t* to consumed token or NULL if not matched
 */
token_t* parser_match_token_call(parser_t* parser, bool (*match)(token_type_e));

/// eat if current token matches specified type or return NULL and add to error_list
token_t* parser_expect_token(parser_t* parser, token_type_e expected_type);

/// eat or return NULL and add a specific error to error_list (not just Expected token: __)
/// - matches based on a specified token_type_e
token_t* parser_expect_token_with_err_code(parser_t* parser, token_type_e expected_type,
                                           error_code_e code);

/// eat or return NULL and add a specific error to error_list (not just Expected token: __)
/// uses a match call that returns bool based on a token_type_e
token_t* parser_expect_token_call(parser_t* parser, bool (*match)(token_type_e), error_code_e code);

// returns true when parser is at EOF
bool parser_eof(parser_t* parser);

/// match helpers

/// returns true when tkn type is builtin
bool token_is_builtin_type(token_type_e t);
/// returns true when tkn type is builtin or an id
bool token_is_builtin_type_or_id(token_type_e t);
/// returns true when tkn type is builtin type mod (box, bag, etc)
bool token_is_builtin_type_mod(token_type_e t);
/// returns true when tkn type is a literal
bool token_is_literal(token_type_e t);

bool token_is_preunary_op(token_type_e t);
bool token_is_binary_op(token_type_e t);
#endif
