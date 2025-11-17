// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "compiler/diagnostics/error_codes.h"
#include "compiler/token.h"

const char* error_message_for_code(error_code_e error_code) {
    static const char* error_messages[ERR__COUNT] = {[ERR_ILLEGAL_IDENTIFER] = "Illegal identifier",
                                                     [ERR_EXPECTED_TOKEN] = "Expected token: ",
                                                     [ERR_EXPECTED_IDENTIFIER] =
                                                         "Expected identifier",
                                                     [ERR_EXPECTED_TYPE] = "Expected type"};
    return error_messages[error_code];
}

const char* error_message_context_for(compiler_error_t* error) {
    if (error->expected_token_type != TOK_NONE) {
        return token_to_string_map()[error->expected_token_type];
    }
    // this can be extended to support additional contexts in the future
    return ""; // empty string, no additional context
}
