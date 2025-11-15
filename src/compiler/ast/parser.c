// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "compiler/ast/parser.h"
#include "compiler/token.h"
#include <stdbool.h>
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

uint32_t precendence_of(token_type_e type) {
    const uint32_t todo = 0; // TODO
    static bool initialized = false;
    static uint32_t map[TOKEN_MAP_SIZE];
    if (!initialized) {
        map[LPAREN] = todo;
        map[RPAREN] = todo;

        map[LBRACE] = todo;
        map[RBRACE] = todo;

        map[LBRACK] = todo;
        map[RBRACK] = todo;

        map[SEMICOLON] = todo;
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

        map[IMPORT] = todo;
        map[KW_SPACE] = todo;

        map[KW_FN] = todo;
        map[KW_MT] = todo;
        map[KW_CT] = todo;
        map[KW_DT] = todo;

        map[KW_COUT] = todo;
        map[KW_CIN] = todo;

        map[KW_BOX] = todo;
        map[KW_BAG] = todo;

        map[KW_MUT] = todo;
        map[KW_REF] = todo;
        map[KW_INT] = todo;
        map[KW_UINT] = todo;
        map[KW_ULONG] = todo;
        map[KW_CHAR] = todo;
        map[KW_FLT] = todo;
        map[KW_DOUB] = todo;
        map[KW_STR] = todo;
        map[KW_BOOL] = todo;
        map[KW_VOID] = todo;
        map[KW_AUTO] = todo;
        map[KW_COMP] = todo;
        map[KW_HIDDEN] = todo;

        map[KW_TEMPLATE] = todo;

        map[KW_ENUM] = todo;

        map[KW_STATIC] = todo;

        map[KW_IF] = todo;
        map[KW_ELSE] = todo;
        map[KW_ELIF] = todo;
        map[KW_WHILE] = todo;
        map[KW_FOR] = todo;
        map[KW_RETURN] = todo;

        map[KW_THIS] = todo;
        map[KW_STRUCT] = todo;
        map[KW_NEW] = 3;

        // literals
        map[SYMBOL] = todo;
        map[CHAR_LIT] = todo;
        map[INT_LIT] = todo;
        map[LONG_LIT] = todo;
        map[DOUB_LIT] = todo;

        map[STR_LIT] = todo;
        map[BOOL_LIT_FALSE] = todo;
        map[BOOL_LIT_TRUE] = todo;

        map[RARROW] = todo;
        map[SCOPE_RES] = 1;
        map[TYPE_MOD] = todo;

        map[ASSIGN_LARROW] = todo;
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
