// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "compiler/ast/parser.h"
#include "compiler/ast/node.h"
#include "compiler/ast/node_arena.h"
#include "compiler/diagnostics/error_codes.h"
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
    static uint32_t map[TOK__NUM];
    if (!initialized) {
        map[TOK_LPAREN] = NONE;
        map[TOK_RPAREN] = NONE;

        map[TOK_LBRACE] = NONE;
        map[TOK_RBRACE] = NONE;

        map[TOK_LBRACK] = NONE;
        map[TOK_RBRACK] = NONE;

        map[TOK_SEMICOLON] = NONE;
        map[TOK_DOT] = 2;
        map[TOK_COMMA] = 17;

        map[TOK_ASSIGN_EQ] = 16;

        map[TOK_PLUS] = 6;
        map[TOK_MINUS] = 6;

        map[TOK_MULT] = 5;
        map[TOK_DIVIDE] = 5;
        map[TOK_MOD] = 5;

        map[TOK_BIT_OR] = 13;
        map[TOK_BIT_AND] = 11;
        map[TOK_BIT_NOT] = 3;
        map[TOK_BIT_XOR] = 12;

        map[TOK_BOOL_NOT] = 3;

        map[TOK_GT] = 9;
        map[TOK_LT] = 9;

        map[TOK_KW_IMPORT] = NONE;
        map[TOK_KW_SPACE] = NONE;

        map[TOK_KW_FN] = NONE;
        map[TOK_KW_MT] = NONE;
        map[TOK_KW_CT] = NONE;
        map[TOK_KW_DT] = NONE;

        map[TOK_KW_COUT] = NONE;
        map[TOK_KW_CIN] = NONE;

        map[TOK_KW_BOX] = NONE;
        map[TOK_KW_BAG] = NONE;

        map[TOK_KW_MUT] = NONE;
        map[TOK_KW_REF] = NONE;
        map[TOK_KW_INT] = NONE;
        map[TOK_KW_UINT] = NONE;
        map[TOK_KW_ULONG] = NONE;
        map[TOK_KW_CHAR] = NONE;
        map[TOK_KW_FLT] = NONE;
        map[TOK_KW_DOUB] = NONE;
        map[TOK_KW_STR] = NONE;
        map[TOK_KW_BOOL] = NONE;
        map[TOK_KW_VOID] = NONE;
        map[TOK_KW_AUTO] = NONE;
        map[TOK_KW_COMP] = NONE;
        map[TOK_KW_HIDDEN] = NONE;

        map[TOK_KW_TEMPLATE] = NONE;

        map[TOK_KW_ENUM] = NONE;

        map[TOK_KW_STATIC] = NONE;

        map[TOK_KW_IF] = NONE;
        map[TOK_KW_ELSE] = NONE;
        map[TOK_KW_ELIF] = NONE;
        map[TOK_KW_WHILE] = NONE;
        map[TOK_KW_FOR] = NONE;
        map[TOK_KW_RETURN] = NONE;

        map[TOK_KW_THIS] = NONE;
        map[TOK_KW_STRUCT] = NONE;
        map[TOK_KW_NEW] = 3;

        // literals
        map[TOK_SYMBOL] = NONE;
        map[TOK_CHAR_LIT] = NONE;
        map[TOK_INT_LIT] = NONE;
        map[TOK_LONG_LIT] = NONE;
        map[TOK_DOUB_LIT] = NONE;

        map[TOK_STR_LIT] = NONE;
        map[TOK_BOOL_LIT_FALSE] = NONE;
        map[TOK_BOOL_LIT_TRUE] = NONE;

        map[TOK_RARROW] = NONE;
        map[TOK_SCOPE_RES] = 1;
        map[TOK_TYPE_MOD] = NONE;

        map[TOK_ASSIGN_LARROW] = NONE;
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

typedef struct {
    vector_t tokens;
    size_t pos;
    size_t end;
} parser_t;

// consume a token
token_t* parser_eat(parser_t* parser) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    if (tkn->sym != TOK_EOF) {
        parser->pos++;
    }
    return tkn;
}

// peek current uneaten token without consuming it
token_t* parser_peek(parser_t* parser) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    return tkn;
}

// see last eaten token
token_t* parser_prev(parser_t* parser) {
    if (parser->pos == 0) {
        return NULL;
    }
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
        if (tkn->sym != TOK_EOF) {
            parser->pos++;
        }
        return tkn;
    }
    return NULL;
}

// eat or return NULL and add to error_list
token_t* parser_expect_token(parser_t* parser, token_type_e expected_type,
                             compiler_error_list_t* error_list) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    if (tkn->sym == expected_type) {
        parser->pos++;
        return tkn;
    }
    compiler_error_list_emplace_expected_token(error_list, tkn, ERR_EXPECTED_TOKEN, expected_type);
    return NULL;
}

// eat or return NULL and add a specific error to error_list (not just Expected token: __)
token_t* parser_expect(parser_t* parser, token_type_e expected_type,
                       compiler_error_list_t* error_list, error_code_e code) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    if (tkn->sym == expected_type) {
        parser->pos++;
        return tkn;
    }
    compiler_error_list_emplace(error_list, tkn, code);
    return NULL;
}

// returns true when parser is at EOF
bool parser_eof(parser_t* parser) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    return tkn->sym == TOK_EOF;
}

// primary function for parsing a file into an ast
ast_t parser_build_ast_from_file(const char* file_name, vector_t token_vec,
                                 compiler_error_list_t* error_list) {
    // position tracking, consuming tokens, etc
    parser_t parser = {.tokens = token_vec, .pos = 0, .end = token_vec.size};

    // init ast & node arena
    ast_t ast;
    ast.arena = ast_node_arena_create_from_token_vec(&token_vec);
    ast.file_name = file_name; // view
    ast.head = ast_node_arena_new_node(&ast.arena, AST_FILE, NULL, 0);

    // TODO, AST building up logic here
    token_t* tkn = NULL; // scratch token
    while (!parser_eof(&parser)) {
        tkn = parser_match(&parser, TOK_INDETERMINATE);
        if (tkn) {
            compiler_error_list_emplace(error_list, tkn, ERR_ILLEGAL_IDENTIFER);
        } else if (parser_match(&parser, TOK_KW_CHAR)) {
            parser_expect(&parser, TOK_SYMBOL, error_list, ERR_EXPECTED_VARIABLE_ID);
        } else {
            parser_eat(&parser);
        }
    }

    return ast;
}

void ast_destroy(ast_t* ast) { ast_node_arena_destroy(&ast->arena); }
