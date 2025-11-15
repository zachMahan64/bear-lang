// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "compiler/ast/parser.h"
#include "compiler/ast/node.h"
#include "compiler/ast/node_arena.h"
#include "compiler/token.h"
#include "containers/vector.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_ASSOCIATIVITY 18
associativity_e associativity_of(uint32_t precedence) {
    static bool initialized = false;
    static associativity_e map[MAX_ASSOCIATIVITY + 1];

    if (!initialized) {
        map[0] = LEFT_TO_RIGHT;
        map[1] = LEFT_TO_RIGHT;
        map[2] = LEFT_TO_RIGHT;
        map[3] = RIGHT_TO_LEFT;
        for (int i = 4; i < 16; i++) {
            map[i] = LEFT_TO_RIGHT;
        }
        map[16] = RIGHT_TO_LEFT;
        map[17] = LEFT_TO_RIGHT;
        map[18] = RIGHT_TO_LEFT;
        initialized = true;
    }
    return map[precedence];
}
// TODO, need to implement some logic to handle operators with multiple variants (unary vs binary
// +/-)
uint32_t precendence_of_operator(token_type_e type) {
    const uint32_t NONE = 0; // TODO, resolve if any of these shouldn't be none
    static bool initialized = false;
    static uint32_t map[TOKEN_MAP_SIZE];
    if (!initialized) {
        map[LPAREN] = NONE;
        map[RPAREN] = NONE;

        map[LBRACE] = NONE;
        map[RBRACE] = NONE;

        map[LBRACK] = NONE;
        map[RBRACK] = NONE;

        map[SEMICOLON] = NONE;
        map[DOT] = 2;
        map[COMMA] = 17;

        map[ASSIGN_EQ] = 16;

        map[PLUS] = 6;
        map[MINUS] = 6;

        map[MULT] = 5;
        map[DIVIDE] = 5;
        map[MOD] = 5;

        map[BIT_OR] = 13;
        map[BIT_AND] = 11;
        map[BIT_NOT] = 3;
        map[BIT_XOR] = 12;

        map[BOOL_NOT] = 3;

        map[GT] = 9;
        map[LT] = 9;

        map[IMPORT] = NONE;
        map[KW_SPACE] = NONE;

        map[KW_FN] = NONE;
        map[KW_MT] = NONE;
        map[KW_CT] = NONE;
        map[KW_DT] = NONE;

        map[KW_COUT] = NONE;
        map[KW_CIN] = NONE;

        map[KW_BOX] = NONE;
        map[KW_BAG] = NONE;

        map[KW_MUT] = NONE;
        map[KW_REF] = NONE;
        map[KW_INT] = NONE;
        map[KW_UINT] = NONE;
        map[KW_ULONG] = NONE;
        map[KW_CHAR] = NONE;
        map[KW_FLT] = NONE;
        map[KW_DOUB] = NONE;
        map[KW_STR] = NONE;
        map[KW_BOOL] = NONE;
        map[KW_VOID] = NONE;
        map[KW_AUTO] = NONE;
        map[KW_COMP] = NONE;
        map[KW_HIDDEN] = NONE;

        map[KW_TEMPLATE] = NONE;

        map[KW_ENUM] = NONE;

        map[KW_STATIC] = NONE;

        map[KW_IF] = NONE;
        map[KW_ELSE] = NONE;
        map[KW_ELIF] = NONE;
        map[KW_WHILE] = NONE;
        map[KW_FOR] = NONE;
        map[KW_RETURN] = NONE;

        map[KW_THIS] = NONE;
        map[KW_STRUCT] = NONE;
        map[KW_NEW] = 3;

        // literals
        map[SYMBOL] = NONE;
        map[CHAR_LIT] = NONE;
        map[INT_LIT] = NONE;
        map[LONG_LIT] = NONE;
        map[DOUB_LIT] = NONE;

        map[STR_LIT] = NONE;
        map[BOOL_LIT_FALSE] = NONE;
        map[BOOL_LIT_TRUE] = NONE;

        map[RARROW] = NONE;
        map[SCOPE_RES] = 1;
        map[TYPE_MOD] = NONE;

        map[ASSIGN_LARROW] = NONE;
        map[STREAM] = 16;

        map[INC] = 2;
        map[DEC] = 2;

        map[LSH] = 7;
        map[RSHL] = 7;
        map[RSHA] = 7;

        map[BOOL_OR] = 15;
        map[BOOL_AND] = 14;

        map[GE] = 9;
        map[LE] = 9;
        map[EQ] = 10;
        map[NE] = 10;

        map[ASSIGN_PLUS_EQ] = 16;
        map[ASSIGN_MINUS_EQ] = 16;
        map[ASSIGN_MULT_EQ] = 16;
        map[ASSIGN_DIV_EQ] = 16;
        map[ASSIGN_MOD_EQ] = 16;

        map[ASSIGN_AND_EQ] = 16;
        map[ASSIGN_OR_EQ] = 16;
        map[ASSIGN_XOR_EQ] = 16;
        map[ASSIGN_LSH_EQ] = 16;
        map[ASSIGN_RSHL_EQ] = 16;
        map[ASSIGN_RSHA_EQ] = 16;

        map[EOF_TKN] = 18;

        // TODO resolve these todos

        initialized = true;
    }
    return map[type];
}

// primary function for parsing a file into an ast
ast_t parser_build_ast_from_file(const char* file_name, vector_t token_vec) {
    // position tracking for consuming tokens
    size_t pos = 0;
    size_t end = token_vec.size;
    // init ast & node arena
    ast_node_arena_t arena = ast_node_arena_create_from_token_vec(&token_vec);
    ast_t ast;
    ast.file_name = file_name; // view
    ast.head = ast_node_arena_new_node(&arena, AST_FILE, NULL, 0);

    // TODO, AST building up logic here

    return ast;
}
