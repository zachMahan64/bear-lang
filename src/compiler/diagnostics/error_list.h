// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef COMPILER_DIAGNOSTICS_ERROR_LIST_H
#define COMPILER_DIAGNOSTICS_ERROR_LIST_H

#include "compiler/diagnostics/error_codes.h"
#include "compiler/token.h"
#include "containers/vector.h"
#include "file_io.h"

/*
 * compiler error containing the token causing the error w/ its built in meta-data (line & col
 * number) as well as an error message
 *
 * the error message should just explain the error while the final terminal output should be built
 * from both the token's data as well as the error message
 */
typedef struct {
    token_t* token;              // view into a tkn whose resources are externally managed
    error_code_e error_code;     // correspond to a type of compilation error
    token_type_e expected_token; // corresponds to the type of an expected token, NONE by default
} compiler_error_t;

/*
 * list of compiler_error_t's with a file_name
 */
typedef struct {
    const src_buffer_t src_buffer; // holds file_name and view into src code inside a buffer
    vector_t list_vec;             // hold type compiler_error_t
} compiler_error_list_t;

// ctor for error_list_t
compiler_error_list_t compiler_error_list_create(src_buffer_t* src_buffer);

// dtor for error_list_t
void compiler_error_list_destroy(compiler_error_list_t* error_list);

// push an error onto the error list
void compiler_error_list_push(compiler_error_list_t* list, const compiler_error_t* compiler_error);

// emplace an error onto the error list
void compiler_error_list_emplace(compiler_error_list_t* list, token_t* token,
                                 error_code_e error_code);

// print out all compiler errors
void compiler_error_list_print_all(const compiler_error_list_t* list);

// check if a compiler_error_list_t is empty
bool compiler_error_list_empty(const compiler_error_list_t* list);

#endif // COMPILER_DIAGNOSTICS_ERROR_LIST_H
