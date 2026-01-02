//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/compile.h"
#include "compiler/ast/printer.h"
#include "compiler/ast/stmt.h"
#include "compiler/debug.h"
#include "compiler/diagnostics/error_list.h"
#include "compiler/lexer.h"
#include "compiler/parser/parse_stmt.h"
#include "compiler/parser/parser.h"
#include "utils/ansi_codes.h"
#include "utils/arena.h"
#include "utils/file_io.h"
#include "utils/vector.h"
#include <stddef.h>
#include <stdio.h>

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

    // if ONLY EOF TKN
    if (vector_size(&tkn_vec) == 1) {
        goto empty_file_clean_up;
    }

#ifdef DEBUG_BUILD
    print_out_src_buffer(&src_buffer);
    print_out_tkn_table(&tkn_vec);
#endif

    // ----------------------------------------------------

    // ---------------------- PARSING ---------------------
    // init error list for error tracking
    compiler_error_list_t error_list = compiler_error_list_create(&src_buffer);
#define PARSER_ARENA_CHUNK_SIZE_BASE 0x20000
#define PARSER_ARENA_CHUNK_SIZE_SCALE_FACTOR 8
    arena_t arena = arena_create(PARSER_ARENA_CHUNK_SIZE_BASE
                                 + (PARSER_ARENA_CHUNK_SIZE_SCALE_FACTOR * src_buffer.size));
    parser_t parser = parser_create(&tkn_vec, &arena, &error_list);
    ast_stmt_t* file_stmt = parse_file(&parser, src_buffer.file_name);

    ansi_init();

#ifdef DEBUG_BUILD
    print_stmt(file_stmt);
#endif
    // ----------------------------------------------------

    // display all comptime errors
    compiler_error_list_print_all(&error_list);

    if (compiler_error_list_empty(&error_list)) {
        printf("successfully compiled: %s'%s'\n%s", ansi_bold_white(), file_name, ansi_reset());
    } else {
        printf("compilation terminated: %s'%s'\n%s", ansi_bold_white(), file_name, ansi_reset());
        error_code = -1;
    }

    // clean up resources
    arena_destroy(&arena);
    compiler_error_list_destroy(&error_list);
empty_file_clean_up:
    vector_destroy(&tkn_vec);
    src_buffer_destroy(&src_buffer);
    return error_code;
}
