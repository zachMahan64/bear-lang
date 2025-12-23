//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/diagnostics/error_codes.h"
#include "compiler/token.h"

const char* error_message_for_code(error_code_e error_code) {
    static const char* error_messages[ERR__COUNT] = {
        [ERR_EXPECTED_IDENTIFER] = "expected identifier",
        [ERR_EXPECTED_TOKEN] = "expected token: ",
        [ERR_EXPECTED_VARIABLE_NAME] = "expected variable name",
        [ERR_EXPECTED_PARAMETER_IDENTIFIER] = "expected parameter name",
        [ERR_EXPECTED_TYPE] = "expected type",
        [ERR_EXPECTED_EXPRESSION] = "expected expression",
        [ERR_EXPECTED_STATEMENT] = "expected statement",
        [ERR_INCOMPLETE_VAR_DECLARATION] = "expected ';' or assignment operator",
        [ERR_EXPECT_GENERIC_OPENER] = "expected '::' or '<' in generic type",
        [ERR_EXPECTED_BASE_TYPE_IN_GENERIC] =
            "templated can only be applied to base types (not &, *, or arrays)",
        [ERR_REDUNDANT_MUT] = "redundant 'mut' qualifier",
        [ERR_MUT_CANNOT_BIND_TO_ARRAYS] = "'mut' qualifier cannot bind to arrays",
        [ERR_NO_LEADING_MUT_FOR_SLICES] = "'mut' qualifier should be: [&mut] for slices",
        [ERR_EXPECTED_DECLARTION] = "expected variable or function declaration",
        [ERR_EXPECTED_DELIM_IN_MODULE_DECL] = "expected '{' or ';' in module declaration",
        [ERR_INVALID_MODULE_NAME] = "invalid module name",
        [ERR_EXTRANEOUS_SEMICOLON] = "extraneous ';'",
        [ERR_EXTRANEOUS_VISIBILITY_MODIFIER] = "extraneous visibility modifier",
        [ERR_CONDITIONAL_MUST_BE_WRAPPED_IN_BRACES] =
            "expected '{' since statement following condition must be wrapped in braces",
        [ERR_MISMATCHED_RPAREN] = "mismatched ')' without an opening '('",
        [ERR_BREAK_STMT_OUTSIDE_OF_LOOP] = "break statement outside of loop"};
    return error_messages[error_code];
}

const char* error_message_context_for(compiler_error_t* error) {
    if (error->expected_token_type != TOK_NONE) {
        return get_token_to_string_map()[error->expected_token_type];
    }
    // this can be extended to support additional contexts in the future
    return ""; // empty string, no additional context
}
