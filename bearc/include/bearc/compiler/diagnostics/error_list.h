//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_DIAGNOSTICS_ERROR_LIST_H
#define COMPILER_DIAGNOSTICS_ERROR_LIST_H

#include "compiler/diagnostics/error_codes.h"
#include "compiler/token.h"
#include "utils/file_io.h"
#include "utils/vector.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * list of compiler_error_t's with a file_name
 */
typedef struct compiler_error_list {
    const src_buffer_t src_buffer; // holds file_name and view into src code inside a buffer
    vector_t list_vec;             // hold type compiler_error_t
} compiler_error_list_t;

/// ctor for error_list_t
compiler_error_list_t compiler_error_list_create(src_buffer_t* src_buffer);

/// dtor for error_list_t
void compiler_error_list_destroy(compiler_error_list_t* error_list);

/// push an error onto the error list
void compiler_error_list_push(compiler_error_list_t* list, const compiler_error_t* compiler_error);

/// emplace an error onto the error list
void compiler_error_list_emplace(compiler_error_list_t* list, token_t* token,
                                 error_code_e error_code);

/// emplace an error onto the error list with an expected_token_type
void compiler_error_list_emplace_expected_token(compiler_error_list_t* list, token_t* token,
                                                error_code_e error_code,
                                                token_type_e expected_tkn_type);

/// print out all compiler errors
void compiler_error_list_print_all(const compiler_error_list_t* list);

/// check if a compiler_error_list_t is empty
bool compiler_error_list_empty(const compiler_error_list_t* list);

/// check a compiler_error_list_t's size
size_t compiler_error_list_count(const compiler_error_list_t* list);

#ifdef __cplusplus
}
#endif

#endif // COMPILER_DIAGNOSTICS_ERROR_LIST_H
