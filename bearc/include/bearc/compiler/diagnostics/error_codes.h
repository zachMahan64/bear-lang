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
    ERR_EXPECTED_IDENTIFER,
    ERR_EXPECTED_PARAMETER_IDENTIFIER,
    ERR_EXPECTED_VARIABLE_NAME,
    ERR_EXPECTED_TYPE,
    ERR_EXPECTED_EXPRESSION,
    ERR_EXPECTED_STATEMENT,
    ERR_INCOMPLETE_VAR_DECLARATION,
    ERR_EXPECT_GENERIC_OPENER,
    ERR_EXPECTED_BASE_TYPE_IN_GENERIC,
    ERR_REDUNDANT_MUT,
    ERR_MUT_CANNOT_BIND_TO_ARRAYS,
    ERR_NO_LEADING_MUT_FOR_SLICES,
    ERR_EXPECTED_DECLARTION,
    ERR_EXPECTED_DELIM_IN_MODULE_DECL,
    ERR_INVALID_MODULE_NAME,
    ERR_EXTRANEOUS_SEMICOLON,
    ERR_EXTRANEOUS_VISIBILITY_MODIFIER,
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
    token_t* start_tkn; // view into a tkn whose resources are externally managed
    token_t* end_tkn;
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
