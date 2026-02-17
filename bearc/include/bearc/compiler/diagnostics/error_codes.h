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
#include "stdbool.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * predefined set of errors, these have associated diagnostics with
 * the functions in this header
 */
typedef enum error_code {
    ERR_NONE = 0,
    ERR_EXPECTED_TOKEN,
    ERR_EXPECTED_IDENTIFER,
    ERR_EXPECTED_LITERAL,
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
    ERR_BODY_MUST_BE_WRAPPED_IN_BRACES,
    ERR_MISMATCHED_RPAREN,
    ERR_BREAK_STMT_OUTSIDE_OF_LOOP,
    ERR_REDUNDANT_COMPT_QUALIFIER,
    ERR_INVALID_GENERIC_PARAMETER,
    ERR_EXPECTED_FN_OR_MT,
    ERR_TOO_MANY_QUALIFICATIONS_ON_FUNCTION,
    ERR_EXPECTED_ASSIGNMENT,
    ERR_INVALID_PATTERN,
    ERR_MUT_QUALIFIER_ON_NON_MT,
    ERR_IMPORTED_FILE_DOES_NOT_EXIST,
    ERR_REDEFINITON,
    NOTE_ORIGINAL_DEF_HERE,
    ERR_INVALID_FUNCTION_PREFIX,
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

// getting if the error is really a note for a given error_code_e
bool is_really_note(error_code_e error_code);

/**
 * gives addition context for appropriate compiler_error_t's
 * this is safe to call on any compiler_error_t, regardless of its error_code
 * primary use is to get this kind of string:
 * Expected token: ->
 * *                **
 * first string /// this function gives this string
 */
const char* error_message_context_for(compiler_error_t* error);

#ifdef __cplusplus
}
#endif

#endif
