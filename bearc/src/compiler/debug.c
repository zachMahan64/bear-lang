//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/debug.h"
#include "compiler/token.h"
#include "stdio.h"
#include "utils/file_io.h"

void print_out_tkn_table(const vector_t* tkn_vec) {
    const char* const* tkn_map = token_to_string_map();
    size_t tkn_map_size = tkn_vec->size;
    puts("                    Lexed tokens");
    puts("==================================================");
    printf("%-15s | %-17s | %-7s \n", "sym", "   line, column", " str value");
    puts("==================================================");
    for (size_t i = 0; i < tkn_map_size; i++) {
        token_t* tkn = (token_t*)vector_at(tkn_vec, i);
        printf("%-15s @ %7zu, %-7zu -> [%.*s]\n", tkn_map[tkn->type], tkn->loc.line, tkn->loc.col,
               (int)tkn->len, tkn->start);
    }
    puts("==================================================");
}

void print_out_src_buffer(const src_buffer_t* src_buffer) {
    printf("\n"
           " Contents of [%s]\n"
           "==================================================\n"
           "%s\n"
           "==================================================\n",
           src_buffer->file_name, src_buffer->data);
}
