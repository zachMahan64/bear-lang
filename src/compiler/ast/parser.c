// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "compiler/ast/parser.h"
#include "compiler/ast/node.h"
#include "compiler/ast/node_arena.h"
#include "compiler/diagnostics/error_list.h"
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
    static uint32_t map[TKN__NUM];
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
        map[BOOL_EQ] = 10;
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

        initialized = true;
    }
    return map[type];
}

typedef struct {
    vector_t tokens;
    size_t pos;
    size_t end;
} parser_t;

// consume a token
token_t* parser_eat(parser_t* parser) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    parser->pos++;
    return tkn;
}

// peek next uneaten token without consuming it
token_t* parser_peek(parser_t* parser) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    return tkn;
}

// see last eaten token
token_t* parser_prev(parser_t* parser) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos - 1);
    return tkn;
}

/*
 * peek then conditionally eat
 * \return token_t* to consumed token or NULL if not matched
 */
token_t* parser_match(parser_t* parser, token_type_e type) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    if (tkn->sym == type) {
        parser->pos++;
        return tkn;
    }
    return NULL;
}

// eat or return NULL and add to error_list
token_t* parser_expect(parser_t* parser, token_type_e type, compiler_error_list_t* error_list);

// primary function for parsing a file into an ast
ast_t parser_build_ast_from_file(const char* file_name, vector_t token_vec) {
    // position tracking, consuming tokens, etc
    parser_t parser = {.tokens = token_vec, .pos = 0, .end = token_vec.size};

    // init ast & node arena
    ast_t ast;
    ast.arena = ast_node_arena_create_from_token_vec(&token_vec);
    ast.file_name = file_name; // view
    ast.head = ast_node_arena_new_node(&ast.arena, AST_FILE, NULL, 0);

    // TODO, AST building up logic here

    return ast;
}

void ast_destroy(ast_t* ast) { ast_node_arena_destroy(&ast->arena); }
