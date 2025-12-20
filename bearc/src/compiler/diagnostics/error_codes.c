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
        [ERR_ILLEGAL_IDENTIFER] = "illegal identifier",
        [ERR_EXPECTED_TOKEN] = "expected token: ",
        [ERR_EXPECTED_VARIABLE_IDENTIFIER] = "expected variable identifier",
        [ERR_EXPECTED_PARAMETER_IDENTIFIER] = "expected parameter identifier",
        [ERR_EXPECTED_TYPE] = "expected type",
        [ERR_EXPECTED_EXPRESSION] = "expected expression",
        [ERR_EXPECTED_STATEMENT] = "expected statement",
        [ERR_INCOMPLETE_VAR_DECLARATION] = "expected ';' or assignment operator",
        [ERR_EXPECT_GENERIC_OPENER] = "expected '::' or '<' in generic type",
        [ERR_EXPECTED_BASE_TYPE_IN_GENERIC] =
            "templated can only be applied to base types (not &, *, or arrays)",
    };
    return error_messages[error_code];
}

const char* error_message_context_for(compiler_error_t* error) {
    if (error->expected_token_type != TOK_NONE) {
        return get_token_to_string_map()[error->expected_token_type];
    }
    // this can be extended to support additional contexts in the future
    return ""; // empty string, no additional context
}
