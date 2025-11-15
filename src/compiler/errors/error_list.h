// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef COMPILER_AST_ERROR_LIST_H
#define COMPILER_AST_ERROR_LIST_H

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
    token_t* token;        // view into a tkn whose resources are externally managed
    const char* error_msg; // should be a string literal
} compiler_error_t;

/*
 * list of compiler_error_t's with a file_name
 */
typedef struct {
    const src_buffer_t src_buffer; // holds file_name and view into src code inside a buffer
    vector_t list_vec;             // hold type compiler_error_t
} error_list_t;

// ctor for error_list_t
error_list_t error_list_create(src_buffer_t src_buffer);

// dtor for error_list_t
void error_list_destroy(error_list_t* error_list);

// push an error onto the error list
void error_list_push(error_list_t* list, const compiler_error_t* compiler_error);

// emplace an error onto the error list
void error_list_emplace(error_list_t* list, token_t* token, const char* error_msg);

#endif // COMPILER_AST_ERROR_LIST_H
