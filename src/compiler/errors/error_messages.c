// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "compiler/errors/error_messages.h"
#include <stdbool.h>

const char* error_message_for(error_code_e error_code) {
    static bool initialized = false;
    static const char* error_messages[ERROR_MESSAGES_NUM];
    if (!initialized) {
        error_messages[ERR_UNRECOGNIZED_SYMBOL] = "Unrecognized symbol";
        initialized = true;
    }
    return error_messages[error_code];
}
