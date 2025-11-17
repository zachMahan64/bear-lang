// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef COMPILER_DIAGNOSTICS_ERROR_CODES_H
#define COMPILER_DIAGNOSTICS_ERROR_CODES_H

typedef enum { ERR_UNRECOGNIZED_SYMBOL, ERR__COUNT } error_code_e;

// getting the error message for a given error_code_e
const char* error_message_for(error_code_e error_code);

#endif
