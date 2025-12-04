// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "compile.h"
#include "compiler/diagnostics/error_list.h"
#include "compiler/lexer.h"
#include "compiler/parser/parser.h"
#include "compiler/token.h"
#include "utils/ansi_codes.h"
#include "utils/file_io.h"
#include "utils/vector.h"
#include <stddef.h>
#include <stdio.h>

// print out contents of src in debug builds
void print_out_src_buffer(src_buffer_t* src_buffer);
// print out lexed token tablw in debug builds
void print_out_tkn_table(vector_t* vec);

int compile_file(const char* file_name) {
    int error_code = 0; // return error code if hit error

    src_buffer_t src_buffer = src_buffer_from_file_create(file_name);

    if (!src_buffer.data) {
        return -1;
    }

    // ---------------------- LEXING ----------------------
    // build up token vector by lexing the source buffer
    vector_t tkn_vec = lexer_tokenize_src_buffer(&src_buffer);
    // detect errors in lexing

    print_out_src_buffer(&src_buffer);
    print_out_tkn_table(&tkn_vec);

    // ----------------------------------------------------

    // ---------------------- PARSING ---------------------
    // init error list for error tracking
    compiler_error_list_t error_list = compiler_error_list_create(&src_buffer);
    ast_t ast = parser_build_ast_from_file(src_buffer.file_name, tkn_vec, &error_list);
    // ----------------------------------------------------

    /* TODO:
     * TOKENIZE -> AST -> SEMANTIC ANALYSIS -> BYTECODE
     * DONE        WIP
     * when we can, just output one unified bytecode file
     * with original_file_name.bvm
     */

    // display all comptime errors
    compiler_error_list_print_all(&error_list);

    if (compiler_error_list_empty(&error_list)) {
        // codegen here
        printf("sucessfully compiled: " ANSI_BOLD "'%s'\n" ANSI_RESET, src_buffer.file_name);
    } else {
        printf("compilation terminated: " ANSI_BOLD "'%s'\n" ANSI_RESET, src_buffer.file_name);
    }

    // clean up resources
    ast_destroy(&ast);
    compiler_error_list_destroy(&error_list);
    vector_destroy(&tkn_vec);
    src_buffer_destroy(&src_buffer);
    return error_code;
}

// private debug helper
void print_out_tkn_table(vector_t* tkn_vec) {
#ifdef DEBUG_BUILD
    const char* const* tkn_map = token_to_string_map();
    size_t tkn_map_size = tkn_vec->size;
    puts("                  Lexed tokens");
    puts("=============================================");
    printf("%-10s | %-17s | %-7s \n", "sym", "   line, column", " str value");
    puts("=============================================");
    for (size_t i = 0; i < tkn_map_size; i++) {
        token_t* tkn = (token_t*)vector_at(tkn_vec, i);
        printf("%-10s @ %7zu, %-7zu -> [%.*s]\n", tkn_map[tkn->sym], tkn->loc.line, tkn->loc.col,
               (int)tkn->length, tkn->start);
    }
    puts("=============================================");
#endif
}

// private debug helper
void print_out_src_buffer(src_buffer_t* src_buffer) {
#ifdef DEBUG_BUILD
    printf("\n"
           " Contents of [%s]\n"
           "=============================================\n"
           "%s\n"
           "=============================================\n",
           src_buffer->file_name, src_buffer->data);
#endif
}
