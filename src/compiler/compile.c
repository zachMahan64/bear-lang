#include "compile.h"
#include "compiler/lexer.h"
#include "compiler/token.h"
#include "containers/vector.h"
#include "file_io.h"
#include "log.h"
#include <stddef.h>
#include <stdio.h>

int compile_file(const char* file_name) {
    int error_code = 0; // return error code if hit error
    src_buffer_t buffer = src_buffer_from_file_create(file_name);
    // vector_t tkn_vec = lexer_naively_by_whitespace_tokenize_src_buffer(&buffer);
    if (!buffer.data) {
        return -1;
    }
    vector_t tkn_vec = lexer_tokenize_src_buffer(&buffer);

    // DEBUG
    printf("Contents of [%s]:\n~~~~~\n%s\n~~~~~~\n", buffer.file_name, buffer.data);

    printf("Lexed tokens:\n");

    const char* const* tkn_map = get_token_to_string_map();
    size_t tkn_map_size = tkn_vec.size;
    for (size_t i = 0; i < tkn_map_size; i++) {
        token_t* tkn = (token_t*)vector_at(&tkn_vec, i);
        printf("%s -> [%.*s]\n", tkn_map[tkn->sym], (int)tkn->length, tkn->start);
    }

    /* TODO:
     * TOKENIZE -> AST -> BYTECODE
     * WIP         NS     WIP
     * when we can, just output one unified bytecode file
     * with original_file_name.bvm
     */

    src_buffer_destroy(&buffer);
    return error_code;
}
