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

        map[TOK_STAR] = 5;
        map[TOK_DIVIDE] = 5;
        map[TOK_MOD] = 5;

        map[TOK_BAR] = 13;
        map[TOK_AMPER] = 11;
        map[TOK_BIT_NOT] = 3;
        map[TOK_BIT_XOR] = 12;

        map[TOK_BOOL_NOT] = 3;

        map[TOK_GT] = 9;
        map[TOK_LT] = 9;

        map[TOK_IMPORT] = NONE;
        map[TOK_SPACE] = NONE;

        map[TOK_FN] = NONE;
        map[TOK_MT] = NONE;
        map[TOK_CT] = NONE;
        map[TOK_DT] = NONE;

        map[TOK_COUT] = NONE;
        map[TOK_CIN] = NONE;

        map[TOK_BOX] = NONE;
        map[TOK_BAG] = NONE;

        map[TOK_MUT] = NONE;
        map[TOK_I32] = NONE;
        map[TOK_U32] = NONE;
        map[TOK_U64] = NONE;
        map[TOK_CHAR] = NONE;
        map[TOK_U8] = NONE;
        map[TOK_F32] = NONE;
        map[TOK_F64] = NONE;
        map[TOK_STR] = NONE;
        map[TOK_BOOL] = NONE;
        map[TOK_VOID] = NONE;
        map[TOK_AUTO] = NONE;
        map[TOK_COMPT] = NONE;
        map[TOK_HID] = NONE;

        map[TOK_TEMPLATE] = NONE;

        map[TOK_ENUM] = NONE;

        map[TOK_STATIC] = NONE;

        map[TOK_IF] = NONE;
        map[TOK_ELSE] = NONE;
        map[TOK_ELIF] = NONE;
        map[TOK_WHILE] = NONE;
        map[TOK_FOR] = NONE;
        map[TOK_RETURN] = NONE;

        map[TOK_THIS] = NONE;
        map[TOK_STRUCT] = NONE;
        map[TOK_NEW] = 3;

        // literals
        map[TOK_IDENTIFIER] = NONE;
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

        map[TOK_ASSIGN_MOVE] = NONE;
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
} parser_t;

// ----------- token consumption primitive functions -------

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

token_t* parser_peek_n(parser_t* parser, size_t n) {
    size_t idx = parser->pos + n;
    if (idx >= parser->tokens.size) {
        return parser_peek(parser); // return EOF
    }
    return vector_at(&parser->tokens, idx);
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
token_t* parser_match_token(parser_t* parser, token_type_e type) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    if (tkn->sym == type) {
        if (tkn->sym != TOK_EOF) {
            parser->pos++;
        }
        return tkn;
    }
    return NULL;
}

//------------- match forward decls --------------
bool parser_match_is_builtin_type(token_type_e t);

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

/*
 * peek then conditionally eat based on a match function call, which takes a token_type_e and
 * returns bool if valid, else false
 * \return token_t* to consumed token or NULL if not matched
 */
token_t* parser_match_token_call(parser_t* parser, bool (*match)(token_type_e)) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    if (match(tkn->sym)) {
        if (tkn->sym != TOK_EOF) {
            parser->pos++;
        }
        return tkn;
    }
    return NULL;
}

// eat if current token matches specified type or return NULL and add to error_list
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
// matches based on a specified token_type_e
token_t* parser_expect_token_with_err_code(parser_t* parser, token_type_e expected_type,
                                           compiler_error_list_t* error_list, error_code_e code) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    if (tkn->sym == expected_type) {
        parser->pos++;
        return tkn;
    }
    compiler_error_list_emplace(error_list, tkn, code);
    return NULL;
}

// eat or return NULL and add a specific error to error_list (not just Expected token: __)
// uses a match call that returns bool based on a token_type_e
token_t* parser_expect_token_call(parser_t* parser, bool (*match)(token_type_e),
                                  compiler_error_list_t* error_list, error_code_e code) {
    token_t* tkn = vector_at(&parser->tokens, parser->pos);
    if (match(tkn->sym)) {
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

// map containing look-ups for builtin types
static const bool parser_builtin_type_map[TOK__NUM] = {
    [TOK_CHAR] = true, [TOK_U8] = true,   [TOK_I8] = true,   [TOK_I32] = true,
    [TOK_U32] = true,  [TOK_I64] = true,  [TOK_U64] = true,  [TOK_F32] = true,
    [TOK_F64] = true,  [TOK_BOOL] = true, [TOK_STR] = true,  [TOK_SPACE] = true,
    [TOK_FN] = true,   [TOK_MT] = true,   [TOK_CT] = true,   [TOK_DT] = true,
    [TOK_AUTO] = true, [TOK_VOID] = true, [TOK_ENUM] = true, [TOK_STRUCT] = true,
};

// match helpers
bool parser_match_is_builtin_type(token_type_e t) { return parser_builtin_type_map[t]; }

// ^^^^^^^^^^^^^^^^ token consumption primitive functions ^^^^^^^^^^^^^^^^^^^^^^^^^^^

// primary function for parsing a file into an ast
ast_t parser_build_ast_from_file(const char* file_name, vector_t token_vec,
                                 compiler_error_list_t* error_list) {
    // position tracking, consuming tokens, etc
    parser_t parser = {.tokens = token_vec, .pos = 0};

    // init ast & node arena
    ast_t ast;
    ast.arena = ast_node_arena_create_from_token_vec(&token_vec);
    ast.file_name = file_name; // view
    ast.head = ast_node_arena_new_node(&ast.arena, AST_FILE, NULL, 0);

    // TODO, AST building up logic here
    // right now this is just some placeholder logic to test the basic parser functions
    token_t* tkn = NULL; // scratch token
    while (!parser_eof(&parser)) {
        tkn = parser_match_token(&parser, TOK_INDETERMINATE);
        if (tkn) {
            compiler_error_list_emplace(error_list, tkn, ERR_ILLEGAL_IDENTIFER);
        }
        // expect id after builtin type
        else if (parser_match_token_call(&parser, &parser_match_is_builtin_type)) {
            parser_expect_token_with_err_code(&parser, TOK_IDENTIFIER, error_list,
                                              ERR_EXPECTED_IDENTIFIER);
        }

        // expect type after rarrow
        else if (parser_match_token(&parser, TOK_RARROW)) {
            parser_expect_token_call(&parser, &parser_match_is_builtin_type, error_list,
                                     ERR_EXPECTED_TYPE);
        }

        // just consume
        else {
            parser_eat(&parser);
        }
    }

    return ast;
}

void ast_destroy(ast_t* ast) { ast_node_arena_destroy(&ast->arena); }
