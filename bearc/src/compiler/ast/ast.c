//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/ast/ast.h"
#include "compiler/lexer.h"
#include "compiler/parser/parse_stmt.h"
#include "utils/file_io.h"

br_ast_t ast_create_from_file(const char* file_name) {
    src_buffer_t src_buffer = src_buffer_from_file_create(file_name);

    compiler_error_list_t error_list = compiler_error_list_create(&src_buffer);

    br_ast_t ast = {{0}, {0}, {0}, NULL, error_list};
    if (!src_buffer.data) {
        return ast;
    }

    // ---------------------- LEXING ----------------------
    // build up token vector by lexing the source buffer
    vector_t tkn_vec = lexer_tokenize_src_buffer(&src_buffer);
    // detect errors in lexing

    // ----------------------------------------------------

    // ---------------------- PARSING ---------------------
    // init error list for error tracking
#define PARSER_ARENA_CHUNK_SIZE_BASE 0x20000
#define PARSER_ARENA_CHUNK_SIZE_SCALE_FACTOR 8
    arena_t arena = arena_create(PARSER_ARENA_CHUNK_SIZE_BASE
                                 + (PARSER_ARENA_CHUNK_SIZE_SCALE_FACTOR * src_buffer.size));
    parser_t parser = parser_create(&tkn_vec, &arena, &ast.error_list);
    ast_stmt_t* file_stmt = parse_file(&parser, src_buffer.file_name);
    ast.file_stmt_root_node = file_stmt;
    ast.src_buffer = src_buffer;
    ast.arena = arena;
    ast.tokens = tkn_vec;
    return ast;
}

void ast_destroy(br_ast_t* ast) {
    arena_destroy(&ast->arena);
    vector_destroy(&ast->tokens);
    src_buffer_destroy(&ast->src_buffer);
    ast->file_stmt_root_node = NULL;
    compiler_error_list_destroy(&ast->error_list);
}
