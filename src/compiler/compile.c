// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "compile.h"
#include "compiler/lexer.h"
#include "compiler/token.h"
#include "containers/vector.h"
#include "file_io.h"
#include "log.h"
#include <stddef.h>
#include <stdio.h>

void print_out_tkn_table(vector_t* tkn_vec) {
    const char* const* tkn_map = get_token_to_string_map();
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
}

int compile_file(const char* file_name) {
    int error_code = 0; // return error code if hit error
    src_buffer_t buffer = src_buffer_from_file_create(file_name);
    // vector_t tkn_vec = lexer_naively_by_whitespace_tokenize_src_buffer(&buffer);
    if (!buffer.data) {
        return -1;
    }
    vector_t tkn_vec = lexer_tokenize_src_buffer(&buffer);
    // DEBUG
    printf("\n"
           " Contents of [%s]\n"
           "=============================================\n"
           "%s\n"
           "=============================================\n",
           buffer.file_name, buffer.data);
    print_out_tkn_table(&tkn_vec);
    /* TODO:
     * TOKENIZE -> AST -> BYTECODE
     * WIP         NS     WIP
     * when we can, just output one unified bytecode file
     * with original_file_name.bvm
     */
    src_buffer_destroy(&buffer);
    return error_code;
}
