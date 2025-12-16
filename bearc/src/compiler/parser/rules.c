//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/parser/rules.h"

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
    static uint32_t map[TOK__NUM];
    if (!initialized) {
        map[TOK_LBRACK] = NONE;
        map[TOK_RBRACK] = NONE;

        map[TOK_DOT] = 2;
        map[TOK_COMMA] = 17;

        map[TOK_ASSIGN_EQ] = 16;

        map[TOK_PLUS] = 6;
        map[TOK_MINUS] = 6;

        map[TOK_STAR] = 5;
        map[TOK_DIVIDE] = 5;
        map[TOK_MODULO] = 5;

        map[TOK_BAR] = 13;
        map[TOK_AMPER] = 11;
        map[TOK_BIT_NOT] = 3;
        map[TOK_BIT_XOR] = 12;

        map[TOK_BOOL_NOT] = 3;

        map[TOK_GT] = 9;
        map[TOK_LT] = 9;

        map[TOK_SCOPE_RES] = 1;
        map[TOK_STREAM] = 16;

        map[TOK_INC] = 2;
        map[TOK_DEC] = 2;

        map[TOK_LSH] = 7;
        map[TOK_RSHL] = 7;
        map[TOK_RSHA] = 7;

        map[TOK_BOOL_OR] = 15;
        map[TOK_BOOL_AND] = 14;

        map[TOK_GE] = 9;
        map[TOK_LE] = 9;
        map[TOK_BOOL_EQ] = 10;
        map[TOK_NE] = 10;

        map[TOK_ASSIGN_PLUS_EQ] = 16;
        map[TOK_ASSIGN_MINUS_EQ] = 16;
        map[TOK_ASSIGN_MULT_EQ] = 16;
        map[TOK_ASSIGN_DIV_EQ] = 16;
        map[TOK_ASSIGN_MOD_EQ] = 16;

        map[TOK_ASSIGN_AND_EQ] = 16;
        map[TOK_ASSIGN_OR_EQ] = 16;
        map[TOK_ASSIGN_XOR_EQ] = 16;
        map[TOK_ASSIGN_LSH_EQ] = 16;
        map[TOK_ASSIGN_RSHL_EQ] = 16;
        map[TOK_ASSIGN_RSHA_EQ] = 16;

        map[TOK_EOF] = 18;

        initialized = true;
    }
    return map[type];
}
