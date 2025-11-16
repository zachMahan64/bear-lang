// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef COMPILER_ERROR_CODES_H
#define COMPILER_ERROR_CODES_H

#define ERROR_CODE_NUM 32

typedef enum { ERR_UNRECOGNIZED_SYMBOL } error_code_e;

// getting the error message for a given error_code_e
const char* error_message_for(error_code_e error_code);

#endif
