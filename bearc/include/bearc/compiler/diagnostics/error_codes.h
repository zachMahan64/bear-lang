//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_DIAGNOSTICS_ERROR_CODES_H
#define COMPILER_DIAGNOSTICS_ERROR_CODES_H

#include "compiler/token.h"

/**
 * predefined set of errors, these have associated diagnostics with
 * the functions in this header
 */
typedef enum {
    ERR_NONE = 0,
    ERR_EXPECTED_TOKEN,
    ERR_ILLEGAL_IDENTIFER,
    ERR_EXPECTED_PARAMETER_IDENTIFIER,
    ERR_EXPECTED_VARIABLE_IDENTIFIER,
    ERR_EXPECTED_TYPE,
    ERR_EXPECTED_EXPRESSION,
    ERR_EXPECTED_STATEMENT,
    ERR_INCOMPLETE_VAR_DECLARATION,
    ERR__COUNT
} error_code_e;

/**
 * compiler error containing the token causing the error w/ its built in meta-data (line & col
 * number) as well as an error message
 *
 * the error message should just explain the error while the final terminal output should be built
 * from both the token's data as well as the error message
 */
typedef struct {
    token_t* token;          // view into a tkn whose resources are externally managed
    error_code_e error_code; // correspond to a type of compilation error
    // corresponds to the type of an expected token, should be NONE by default
    token_type_e expected_token_type;
} compiler_error_t;

// getting the error message for a given error_code_e
const char* error_message_for_code(error_code_e error_code);

/**
 * gives addition context for appropriate compiler_error_t's
 * this is safe to call on any compiler_error_t, regardless of its error_code
 * primary use is to get this kind of string:
 * Expected token: ->
 * *                **
 * first string /// this function gives this string
 */
const char* error_message_context_for(compiler_error_t* error);

#endif
