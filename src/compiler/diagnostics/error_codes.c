// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "compiler/diagnostics/error_codes.h"
#include <stdbool.h>

const char* error_message_for(error_code_e error_code) {
    static const char* error_messages[ERR__COUNT] = {
        [ERR_UNRECOGNIZED_SYMBOL] = "Unrecognized symbol",
    };
    return error_messages[error_code];
}
