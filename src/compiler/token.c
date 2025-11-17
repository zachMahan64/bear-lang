// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "token.h"
#include "containers/strimap.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/**
 * returns a view into statically allocated map of char -> token_type_e
 */
const char* get_char_to_token_map(void) {
    static bool initialized = false;
    static char char_to_token_map[TOKEN_CHAR_TO_TOKEN_MAP_SIZE]; // ASCII lookup
    if (!initialized) {
        // delimiters
        char_to_token_map['('] = TOK_LPAREN;
        char_to_token_map[')'] = TOK_RPAREN;
        char_to_token_map['{'] = TOK_LBRACE;
        char_to_token_map['}'] = TOK_RBRACE;
        char_to_token_map['['] = TOK_LBRACK;
        char_to_token_map[']'] = TOK_RBRACK;

        // punctuation
        char_to_token_map[';'] = TOK_SEMICOLON;
        char_to_token_map['.'] = TOK_DOT;
        char_to_token_map[','] = TOK_COMMA;

        // assignment
        char_to_token_map['='] = TOK_ASSIGN_EQ;

        // arithmetic
        char_to_token_map['+'] = TOK_PLUS;
        char_to_token_map['-'] = TOK_MINUS;
        char_to_token_map['*'] = TOK_MULT;
        char_to_token_map['/'] = TOK_DIVIDE;
        char_to_token_map['%'] = TOK_MOD;

        // bitwise
        char_to_token_map['|'] = TOK_BIT_OR;
        char_to_token_map['&'] = TOK_BIT_AND;
        char_to_token_map['~'] = TOK_BIT_NOT;
        char_to_token_map['^'] = TOK_BIT_XOR;

        // boolean
        char_to_token_map['!'] = TOK_BOOL_NOT;

        // comparison
        char_to_token_map['>'] = TOK_GT;
        char_to_token_map['<'] = TOK_LT;

        initialized = true;
    }
    return char_to_token_map;
}

// returns a view into statically allocated map of string -> token_type_e
const strimap_t* get_string_to_token_strimap(void) {
    static bool initialized = false;
    static strimap_t map;

    if (!initialized) {
        map = strimap_create(TOKEN_STRING_TO_TOKEN_MAP_SIZE); // make large to reduce hash conflicts

        // bool literals
        strimap_insert(&map, "true", TOK_BOOL_LIT_TRUE);
        strimap_insert(&map, "false", TOK_BOOL_LIT_FALSE);

        // file
        strimap_insert(&map, "import", TOK_IMPORT);

        // keywords
        strimap_insert(&map, "space", TOK_SPACE);
        strimap_insert(&map, "fn", TOK_FN);
        strimap_insert(&map, "mt", TOK_MT);
        strimap_insert(&map, "cout", TOK_COUT);
        strimap_insert(&map, "cin", TOK_CIN);
        strimap_insert(&map, "box", TOK_BOX);
        strimap_insert(&map, "bag", TOK_BAG);
        strimap_insert(&map, "mut", TOK_MUT);
        strimap_insert(&map, "ref", TOK_REF);
        strimap_insert(&map, "int", TOK_INT);
        strimap_insert(&map, "uint", TOK_UINT);
        strimap_insert(&map, "char", TOK_CHAR);
        strimap_insert(&map, "byte", TOK_BYTE);
        strimap_insert(&map, "flt", TOK_FLT);
        strimap_insert(&map, "doub", TOK_DOUB);
        strimap_insert(&map, "long", TOK_LONG);
        strimap_insert(&map, "ulong", TOK_ULONG);
        strimap_insert(&map, "str", TOK_STR);
        strimap_insert(&map, "bool", TOK_BOOL);
        strimap_insert(&map, "void", TOK_VOID);
        strimap_insert(&map, "auto", TOK_AUTO);
        strimap_insert(&map, "static", TOK_STATIC);
        strimap_insert(&map, "comp", TOK_COMP);
        strimap_insert(&map, "hid", TOK_HID);
        strimap_insert(&map, "template", TOK_TEMPLATE);
        strimap_insert(&map, "enum", TOK_ENUM);

        // control flow
        strimap_insert(&map, "if", TOK_IF);
        strimap_insert(&map, "else", TOK_ELSE);
        strimap_insert(&map, "elif", TOK_ELIF);
        strimap_insert(&map, "while", TOK_WHILE);
        strimap_insert(&map, "for", TOK_FOR);
        strimap_insert(&map, "return", TOK_RETURN);

        // structures
        strimap_insert(&map, "this", TOK_THIS);
        strimap_insert(&map, "struct", TOK_STRUCT);
        strimap_insert(&map, "new", TOK_NEW);

        // operators / symbols (multi-char tokens)
        strimap_insert(&map, "->", TOK_RARROW);
        strimap_insert(&map, "..", TOK_SCOPE_RES);
        strimap_insert(&map, "::", TOK_TYPE_MOD);
        strimap_insert(&map, "<-", TOK_ASSIGN_LARROW);
        strimap_insert(&map, "<<-", TOK_STREAM);
        strimap_insert(&map, "++", TOK_INC);
        strimap_insert(&map, "--", TOK_DEC);
        strimap_insert(&map, "<<", TOK_LSH);
        strimap_insert(&map, ">>", TOK_RSHL);
        strimap_insert(&map, ">>>", TOK_RSHA);
        strimap_insert(&map, "||", TOK_BOOL_OR);
        strimap_insert(&map, "&&", TOK_BOOL_AND);
        strimap_insert(&map, ">=", TOK_GE);
        strimap_insert(&map, "<=", TOK_LE);
        strimap_insert(&map, "==", TOK_BOOL_EQ);
        strimap_insert(&map, "!=", TOK_NE);

        // compound assignment operators
        strimap_insert(&map, "+=", TOK_ASSIGN_PLUS_EQ);
        strimap_insert(&map, "-=", TOK_ASSIGN_MINUS_EQ);
        strimap_insert(&map, "*=", TOK_ASSIGN_MULT_EQ);
        strimap_insert(&map, "/=", TOK_ASSIGN_DIV_EQ);
        strimap_insert(&map, "%=", TOK_ASSIGN_MOD_EQ);
        strimap_insert(&map, "&=", TOK_ASSIGN_AND_EQ);
        strimap_insert(&map, "|=", TOK_ASSIGN_OR_EQ);
        strimap_insert(&map, "^=", TOK_ASSIGN_XOR_EQ);
        strimap_insert(&map, "<<=", TOK_ASSIGN_LSH_EQ);
        strimap_insert(&map, ">>=", TOK_ASSIGN_RSHL_EQ);
        strimap_insert(&map, ">>>=", TOK_ASSIGN_RSHA_EQ);

        initialized = true;
    }

    return &map;
}

/**
 * returns a view into statically allocated map of token_type_e -> string
 */
const char* const* token_to_string_map(void) {
    static bool initialized = false;
    static const char* map[TOK__NUM] = {0};

    if (!initialized) {
        map[TOK_NONE] = "";
        map[TOK_INDETERMINATE] = "INDETER.";

        // mono-char tokens
        map[TOK_LPAREN] = "(";
        map[TOK_RPAREN] = ")";
        map[TOK_LBRACE] = "{";
        map[TOK_RBRACE] = "}";
        map[TOK_LBRACK] = "[";
        map[TOK_RBRACK] = "]";

        // punctuation
        map[TOK_SEMICOLON] = ";";
        map[TOK_DOT] = ".";
        map[TOK_COMMA] = ",";

        // assignment / operators
        map[TOK_ASSIGN_EQ] = "=";
        map[TOK_PLUS] = "+";
        map[TOK_MINUS] = "-";
        map[TOK_MULT] = "*";
        map[TOK_DIVIDE] = "/";
        map[TOK_MOD] = "%";

        // bitwise
        map[TOK_BIT_OR] = "|";
        map[TOK_BIT_AND] = "&";
        map[TOK_BIT_NOT] = "~";
        map[TOK_BIT_XOR] = "^";

        // boolean
        map[TOK_BOOL_NOT] = "!";

        // comparison
        map[TOK_GT] = ">";
        map[TOK_LT] = "<";

        // file / keywords
        map[TOK_IMPORT] = "import";
        map[TOK_SPACE] = "space";
        map[TOK_FN] = "fn";
        map[TOK_MT] = "mt";
        map[TOK_CT] = "ct";
        map[TOK_DT] = "dt";
        map[TOK_COUT] = "cout";
        map[TOK_CIN] = "cin";
        map[TOK_BOX] = "box";
        map[TOK_BAG] = "bag";
        map[TOK_MUT] = "mut";
        map[TOK_REF] = "ref";
        map[TOK_INT] = "int";
        map[TOK_UINT] = "uint";
        map[TOK_LONG] = "long";
        map[TOK_ULONG] = "ulong";
        map[TOK_CHAR] = "char";
        map[TOK_BYTE] = "byte";
        map[TOK_FLT] = "flt";
        map[TOK_DOUB] = "doub";
        map[TOK_STR] = "str";
        map[TOK_BOOL] = "bool";
        map[TOK_VOID] = "void";
        map[TOK_AUTO] = "auto";
        map[TOK_STATIC] = "static";
        map[TOK_COMP] = "comp";
        map[TOK_HID] = "hidden";
        map[TOK_TEMPLATE] = "template";

        map[TOK_IF] = "if";
        map[TOK_ELSE] = "else";
        map[TOK_ELIF] = "elif";
        map[TOK_WHILE] = "while";
        map[TOK_FOR] = "for";
        map[TOK_RETURN] = "return";

        // structures
        map[TOK_THIS] = "this";
        map[TOK_STRUCT] = "struct";
        map[TOK_NEW] = "new";

        // variable / literal types
        map[TOK_IDENTIFIER] = "id";
        map[TOK_CHAR_LIT] = "char_lit";
        map[TOK_INT_LIT] = "int_lit";
        map[TOK_DOUB_LIT] = "doub_lit";
        map[TOK_STR_LIT] = "str_lit";
        map[TOK_BOOL_LIT_TRUE] = "true_lit";
        map[TOK_BOOL_LIT_FALSE] = "false_lit";

        // special punctuation
        map[TOK_RARROW] = "->";
        map[TOK_SCOPE_RES] = "..";
        map[TOK_TYPE_MOD] = "::";

        // operators
        map[TOK_ASSIGN_LARROW] = "<-";
        map[TOK_STREAM] = "<<-";
        map[TOK_INC] = "++";
        map[TOK_DEC] = "--";
        map[TOK_LSH] = "<<";
        map[TOK_RSHL] = ">>";
        map[TOK_RSHA] = ">>>";
        map[TOK_BOOL_OR] = "||";
        map[TOK_BOOL_AND] = "&&";
        map[TOK_GE] = ">=";
        map[TOK_LE] = "<=";
        map[TOK_BOOL_EQ] = "==";
        map[TOK_NE] = "!=";

        // --------------- compound assignment -----------------
        // arithmetic
        map[TOK_ASSIGN_PLUS_EQ] = "+=";
        map[TOK_ASSIGN_MINUS_EQ] = "-=";
        map[TOK_ASSIGN_MULT_EQ] = "*=";
        map[TOK_ASSIGN_DIV_EQ] = "/=";
        map[TOK_ASSIGN_MOD_EQ] = "%=";

        // bitwise
        map[TOK_ASSIGN_AND_EQ] = "&=";
        map[TOK_ASSIGN_OR_EQ] = "|=";
        map[TOK_ASSIGN_XOR_EQ] = "^=";
        map[TOK_ASSIGN_LSH_EQ] = "<<=";
        map[TOK_ASSIGN_RSHL_EQ] = ">>=";
        map[TOK_ASSIGN_RSHA_EQ] = ">>>=";

        // EOF
        map[TOK_EOF] = "eof";
        map[TOK_LEX_ERROR_EMPTY_TOKEN] = "err_empty_token";

        initialized = true;
    }
    return map;
}

/**
 * get map that contains always mono-char : token
 */
const char* get_always_one_char_to_token_map(void) {
    static bool initialized = false;
    static char char_to_token_map[TOKEN_CHAR_TO_TOKEN_MAP_SIZE]; // ASCII lookup
    if (!initialized) {
        // delimiters
        char_to_token_map['('] = TOK_LPAREN;
        char_to_token_map[')'] = TOK_RPAREN;
        char_to_token_map['{'] = TOK_LBRACE;
        char_to_token_map['}'] = TOK_RBRACE;
        char_to_token_map['['] = TOK_LBRACK;
        char_to_token_map[']'] = TOK_RBRACK;

        // punctuation
        char_to_token_map[';'] = TOK_SEMICOLON;
        char_to_token_map[','] = TOK_COMMA;

        initialized = true;
    }
    return char_to_token_map;
}

const char* get_first_char_in_multichar_operator_token_map(void) {
    static bool initialized = false;
    static char char_to_token_map[TOKEN_CHAR_TO_TOKEN_MAP_SIZE]; // ASCII lookup
    if (!initialized) {
        // punctuation
        char_to_token_map['.'] = TOK_DOT;
        char_to_token_map[':'] = TOK_COLON;

        // assignment
        char_to_token_map['='] = TOK_ASSIGN_EQ;

        // arithmetic
        char_to_token_map['+'] = TOK_PLUS;
        char_to_token_map['-'] = TOK_MINUS;
        char_to_token_map['*'] = TOK_MULT;
        char_to_token_map['/'] = TOK_DIVIDE;
        char_to_token_map['%'] = TOK_MOD;

        // bitwise
        char_to_token_map['|'] = TOK_BIT_OR;
        char_to_token_map['&'] = TOK_BIT_AND;
        char_to_token_map['~'] = TOK_BIT_NOT;
        char_to_token_map['^'] = TOK_BIT_XOR;

        // boolean
        char_to_token_map['!'] = TOK_BOOL_NOT;

        // comparison
        char_to_token_map['>'] = TOK_GT;
        char_to_token_map['<'] = TOK_LT;

        initialized = true;
    }
    return char_to_token_map; // return the map itself for faster look-up
}

// helper
token_type_e token_determine_token_type_for_fixed_symbols(const char* start, size_t length);
// helper
void token_check_if_valid_literal_and_set_value(token_t* tkn);
// helper
void token_check_if_valid_symbol_and_set_sym(token_t* tkn);

/**
 * Builds token according to a starting ptr and length into src as well as an src_loc_t that
 * indicates row/col number for debugging purposes. This function assumes that the lexer has already
 * correct determined the string that needs to be tokenized.
 */
token_t token_build(const char* start, size_t length, src_loc_t* loc) {
    token_t tkn;
    // init provided values
    tkn.start = start;
    tkn.length = length;
    tkn.loc = *loc;
    // now determine most compilcated fields:
    // keywords and reserved symbols

    tkn.sym = token_determine_token_type_for_fixed_symbols(
        start,
        length); // will be set to INDETERMINATE if the token could not be resolved as a fixed
                 // symbol

    if (tkn.sym == TOK_INDETERMINATE) {
        token_check_if_valid_literal_and_set_value(
            &tkn); // will appropriately set literal symbol and values, but will leave sym as
                   // INDETERMINATE if no pattern was matched
    }

    if (tkn.sym == TOK_INDETERMINATE) {
        token_check_if_valid_symbol_and_set_sym(&tkn);
    }
    return tkn;
}

/**
 * helper that looks up tokens for fixed symbols (i.e keywords and operators), impls basic logic
 * using different look ups for optimization
 */
token_type_e token_determine_token_type_for_fixed_symbols(const char* start, size_t length) {
    const char* char_to_token_map = get_char_to_token_map();
    if (length == 1) {
        return char_to_token_map[(
            unsigned char)start[0]]; // return token indexed at the mono-char literal, any
                                     // invalid entry will be automatically '\0' since the
                                     // map was statically initialized and token
                                     // INDETERMINATE = 0
    }
    const strimap_t* string_to_token_map = get_string_to_token_strimap();
    token_type_e* tkn_type_ptr_from_multi_char_map =
        (token_type_e*)strimap_viewn(string_to_token_map, start, length);

    if (tkn_type_ptr_from_multi_char_map != NULL) {
        return *tkn_type_ptr_from_multi_char_map; // derefence and return because it
                                                  // was valid and thus found
    }
    return TOK_INDETERMINATE; // because strimap_viewn return NULL, it was thus not found and
                              // INDETERMINATE
}

void token_check_if_valid_literal_and_set_value(token_t* tkn) {
    if (!tkn || tkn->length == 0) {
        if (tkn) {
            tkn->sym = TOK_LEX_ERROR_EMPTY_TOKEN;
        }
        return;
    }

    const char* str = tkn->start;
    size_t len = tkn->length;

    // ~~~ CHAR literal: 'a' or escaped like '\n' ~~~
    if (len >= 3 && str[0] == '\'' && str[len - 1] == '\'') {
        char c;
        if (str[1] == '\\' && len >= 4) { // escaped char
            switch (str[2]) {
            case 'n':
                c = '\n';
                break;
            case 't':
                c = '\t';
                break;
            case '\\':
                c = '\\';
                break;
            case '\'':
                c = '\'';
                break;
            case '"':
                c = '\"';
                break;
            case '0':
                c = '\0';
                break;
            default:
                tkn->sym = TOK_INDETERMINATE;
                return;
            }
        } else if (len == 3) { // simple char
            c = str[1];
        } else {
            tkn->sym = TOK_INDETERMINATE;
            return;
        }
        tkn->sym = TOK_CHAR_LIT;
        tkn->val.character = c;
        return;
    }

    // ~~~ STRING literal: "..." ~~~
    if (len >= 2 && str[0] == '"' && str[len - 1] == '"') {
        tkn->sym = TOK_STR_LIT;
        // value handling / escape sequences deferred
        return;
    }

    // ~~~ NUMERIC LITERALS ~~~
    // copy to temporary buffer and null-terminate
    char buf[64]; // may need to adjust very long numbers
    if (len >= sizeof(buf)) {
        tkn->sym = TOK_INDETERMINATE;
        return;
    }
    memcpy(buf, str, len);
    buf[len] = '\0';

    // INT, use a int literal unless it would overflow
    errno = 0;
    char* endptr = NULL;
    long long integral_val = strtoll(buf, &endptr, 0);
    if (endptr != buf && errno == 0) {
        while (isspace((unsigned char)*endptr)) {
            endptr++;
        }
        if (*endptr == '\0') {
            if (integral_val > INT_MAX || integral_val < INT_MIN) {
                tkn->sym = TOK_LONG_LIT;
                tkn->val.integral = integral_val;
            } else {
                tkn->sym = TOK_INT_LIT;
                tkn->val.integral = integral_val;
            }
            return;
        }
    }

    // FLOATING, default to double literal
    errno = 0;
    endptr = NULL;
    double floating_val = strtod(buf, &endptr);
    if (endptr != buf && errno == 0) {
        while (isspace((unsigned char)*endptr)) {
            endptr++;
        }
        if (*endptr == '\0') {
            tkn->sym = TOK_DOUB_LIT;
            tkn->val.floating = floating_val;
            return;
        }
    }

    tkn->sym = TOK_INDETERMINATE;
}

/**
 * check if a token is a valid variable/function name (a "symbol")
 */
void token_check_if_valid_symbol_and_set_sym(token_t* tkn) {
    // token should never have size zero
    if (tkn->start[0] >= '0' && tkn->start[0] <= '9') {
        tkn->sym = TOK_INDETERMINATE;
        return;
    }
    if (tkn->length <= 0) {
        tkn->sym = TOK_LEX_ERROR_EMPTY_TOKEN;
        return;
    }
    for (size_t i = 0; i < tkn->length; i++) {
        if (!isalnum(tkn->start[i]) && tkn->start[i] != '_') {
            tkn->sym = TOK_INDETERMINATE;
            return;
        }
    }
    tkn->sym = TOK_IDENTIFIER;
}
