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
        char_to_token_map['('] = LPAREN;
        char_to_token_map[')'] = RPAREN;
        char_to_token_map['{'] = LBRACE;
        char_to_token_map['}'] = RBRACE;
        char_to_token_map['['] = LBRACK;
        char_to_token_map[']'] = RBRACK;

        // punctuation
        char_to_token_map[';'] = SEMICOLON;
        char_to_token_map['.'] = DOT;
        char_to_token_map[','] = COMMA;

        // assignment
        char_to_token_map['='] = ASSIGN_EQ;

        // arithmetic
        char_to_token_map['+'] = PLUS;
        char_to_token_map['-'] = MINUS;
        char_to_token_map['*'] = MULT;
        char_to_token_map['/'] = DIVIDE;
        char_to_token_map['%'] = MOD;

        // bitwise
        char_to_token_map['|'] = BIT_OR;
        char_to_token_map['&'] = BIT_AND;
        char_to_token_map['~'] = BIT_NOT;
        char_to_token_map['^'] = BIT_XOR;

        // boolean
        char_to_token_map['!'] = BOOL_NOT;

        // comparison
        char_to_token_map['>'] = GT;
        char_to_token_map['<'] = LT;

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
        strimap_insert(&map, "true", BOOL_LIT_TRUE);
        strimap_insert(&map, "false", BOOL_LIT_FALSE);

        // file
        strimap_insert(&map, "import", IMPORT);

        // keywords
        strimap_insert(&map, "space", KW_SPACE);
        strimap_insert(&map, "fn", KW_FN);
        strimap_insert(&map, "mt", KW_MT);
        strimap_insert(&map, "cout", KW_COUT);
        strimap_insert(&map, "cin", KW_CIN);
        strimap_insert(&map, "box", KW_BOX);
        strimap_insert(&map, "bag", KW_BAG);
        strimap_insert(&map, "mut", KW_MUT);
        strimap_insert(&map, "ref", KW_REF);
        strimap_insert(&map, "int", KW_INT);
        strimap_insert(&map, "uint", KW_UINT);
        strimap_insert(&map, "char", KW_CHAR);
        strimap_insert(&map, "flt", KW_FLT);
        strimap_insert(&map, "doub", KW_DOUB);
        strimap_insert(&map, "long", KW_LONG);
        strimap_insert(&map, "ulong", KW_ULONG);
        strimap_insert(&map, "str", KW_STR);
        strimap_insert(&map, "bool", KW_BOOL);
        strimap_insert(&map, "void", KW_VOID);
        strimap_insert(&map, "auto", KW_AUTO);
        strimap_insert(&map, "static", KW_STATIC);
        strimap_insert(&map, "comp", KW_COMP);
        strimap_insert(&map, "hidden", KW_HIDDEN);
        strimap_insert(&map, "template", KW_TEMPLATE);
        strimap_insert(&map, "enum", KW_ENUM);

        // control flow
        strimap_insert(&map, "if", KW_IF);
        strimap_insert(&map, "else", KW_ELSE);
        strimap_insert(&map, "elif", KW_ELIF);
        strimap_insert(&map, "while", KW_WHILE);
        strimap_insert(&map, "for", KW_FOR);
        strimap_insert(&map, "return", KW_RETURN);

        // structures
        strimap_insert(&map, "this", KW_THIS);
        strimap_insert(&map, "struct", KW_STRUCT);
        strimap_insert(&map, "new", KW_NEW);

        // operators / symbols (multi-char tokens)
        strimap_insert(&map, "->", RARROW);
        strimap_insert(&map, "..", SCOPE_RES);
        strimap_insert(&map, "::", TYPE_MOD);
        strimap_insert(&map, "<-", ASSIGN_LARROW);
        strimap_insert(&map, "<<-", STREAM);
        strimap_insert(&map, "++", INC);
        strimap_insert(&map, "--", DEC);
        strimap_insert(&map, "<<", LSH);
        strimap_insert(&map, ">>", RSHL);
        strimap_insert(&map, ">>>", RSHA);
        strimap_insert(&map, "||", BOOL_OR);
        strimap_insert(&map, "&&", BOOL_AND);
        strimap_insert(&map, ">=", GE);
        strimap_insert(&map, "<=", LE);
        strimap_insert(&map, "==", BOOL_EQ);
        strimap_insert(&map, "!=", NE);

        // compound assignment operators
        strimap_insert(&map, "+=", ASSIGN_PLUS_EQ);
        strimap_insert(&map, "-=", ASSIGN_MINUS_EQ);
        strimap_insert(&map, "*=", ASSIGN_MULT_EQ);
        strimap_insert(&map, "/=", ASSIGN_DIV_EQ);
        strimap_insert(&map, "%=", ASSIGN_MOD_EQ);
        strimap_insert(&map, "&=", ASSIGN_AND_EQ);
        strimap_insert(&map, "|=", ASSIGN_OR_EQ);
        strimap_insert(&map, "^=", ASSIGN_XOR_EQ);
        strimap_insert(&map, "<<=", ASSIGN_LSH_EQ);
        strimap_insert(&map, ">>=", ASSIGN_RSHL_EQ);
        strimap_insert(&map, ">>>=", ASSIGN_RSHA_EQ);

        initialized = true;
    }

    return &map;
}

/**
 * returns a view into statically allocated map of token_type_e -> string
 */
const char* const* token_to_string_map(void) {
    static bool initialized = false;
    static const char* map[TKN__NUM] = {0};

    if (!initialized) {
        map[NONE] = "";
        map[INDETERMINATE] = "INDETER.";

        // mono-char tokens
        map[LPAREN] = "(";
        map[RPAREN] = ")";
        map[LBRACE] = "{";
        map[RBRACE] = "}";
        map[LBRACK] = "[";
        map[RBRACK] = "]";

        // punctuation
        map[SEMICOLON] = ";";
        map[DOT] = ".";
        map[COMMA] = ",";

        // assignment / operators
        map[ASSIGN_EQ] = "=";
        map[PLUS] = "+";
        map[MINUS] = "-";
        map[MULT] = "*";
        map[DIVIDE] = "/";
        map[MOD] = "%";

        // bitwise
        map[BIT_OR] = "|";
        map[BIT_AND] = "&";
        map[BIT_NOT] = "~";
        map[BIT_XOR] = "^";

        // boolean
        map[BOOL_NOT] = "!";

        // comparison
        map[GT] = ">";
        map[LT] = "<";

        // file / keywords
        map[IMPORT] = "import";
        map[KW_SPACE] = "space";
        map[KW_FN] = "fn";
        map[KW_MT] = "mt";
        map[KW_CT] = "ct";
        map[KW_DT] = "dt";
        map[KW_COUT] = "cout";
        map[KW_CIN] = "cin";
        map[KW_BOX] = "box";
        map[KW_BAG] = "bag";
        map[KW_MUT] = "mut";
        map[KW_REF] = "ref";
        map[KW_INT] = "int";
        map[KW_UINT] = "uint";
        map[KW_LONG] = "long";
        map[KW_ULONG] = "ulong";
        map[KW_CHAR] = "char";
        map[KW_FLT] = "flt";
        map[KW_DOUB] = "doub";
        map[KW_STR] = "str";
        map[KW_BOOL] = "bool";
        map[KW_VOID] = "void";
        map[KW_AUTO] = "auto";
        map[KW_STATIC] = "static";
        map[KW_COMP] = "comp";
        map[KW_HIDDEN] = "hidden";
        map[KW_TEMPLATE] = "template";

        map[KW_IF] = "if";
        map[KW_ELSE] = "else";
        map[KW_ELIF] = "elif";
        map[KW_WHILE] = "while";
        map[KW_FOR] = "for";
        map[KW_RETURN] = "return";

        // structures
        map[KW_THIS] = "this";
        map[KW_STRUCT] = "struct";
        map[KW_NEW] = "new";

        // variable / literal types
        map[SYMBOL] = "symbol";
        map[CHAR_LIT] = "char_lit";
        map[INT_LIT] = "int_lit";
        map[DOUB_LIT] = "doub_lit";
        map[STR_LIT] = "str_lit";
        map[BOOL_LIT_TRUE] = "true_lit";
        map[BOOL_LIT_FALSE] = "false_lit";

        // special punctuation
        map[RARROW] = "->";
        map[SCOPE_RES] = "..";
        map[TYPE_MOD] = "::";

        // operators
        map[ASSIGN_LARROW] = "<-";
        map[STREAM] = "<<-";
        map[INC] = "++";
        map[DEC] = "--";
        map[LSH] = "<<";
        map[RSHL] = ">>";
        map[RSHA] = ">>>";
        map[BOOL_OR] = "||";
        map[BOOL_AND] = "&&";
        map[GE] = ">=";
        map[LE] = "<=";
        map[BOOL_EQ] = "==";
        map[NE] = "!=";

        // --------------- compound assignment -----------------
        // arithmetic
        map[ASSIGN_PLUS_EQ] = "+=";
        map[ASSIGN_MINUS_EQ] = "-=";
        map[ASSIGN_MULT_EQ] = "*=";
        map[ASSIGN_DIV_EQ] = "/=";
        map[ASSIGN_MOD_EQ] = "%=";

        // bitwise
        map[ASSIGN_AND_EQ] = "&=";
        map[ASSIGN_OR_EQ] = "|=";
        map[ASSIGN_XOR_EQ] = "^=";
        map[ASSIGN_LSH_EQ] = "<<=";
        map[ASSIGN_RSHL_EQ] = ">>=";
        map[ASSIGN_RSHA_EQ] = ">>>=";

        // EOF
        map[EOF_TKN] = "eof";
        map[LEX_ERROR_EMPTY_TOKEN] = "err_empty_token";

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
        char_to_token_map['('] = LPAREN;
        char_to_token_map[')'] = RPAREN;
        char_to_token_map['{'] = LBRACE;
        char_to_token_map['}'] = RBRACE;
        char_to_token_map['['] = LBRACK;
        char_to_token_map[']'] = RBRACK;

        // punctuation
        char_to_token_map[';'] = SEMICOLON;
        char_to_token_map[','] = COMMA;

        initialized = true;
    }
    return char_to_token_map;
}

const char* get_first_char_in_multichar_operator_token_map(void) {
    static bool initialized = false;
    static char char_to_token_map[TOKEN_CHAR_TO_TOKEN_MAP_SIZE]; // ASCII lookup
    if (!initialized) {
        // punctuation
        char_to_token_map['.'] = DOT;
        char_to_token_map[':'] = COLON;

        // assignment
        char_to_token_map['='] = ASSIGN_EQ;

        // arithmetic
        char_to_token_map['+'] = PLUS;
        char_to_token_map['-'] = MINUS;
        char_to_token_map['*'] = MULT;
        char_to_token_map['/'] = DIVIDE;
        char_to_token_map['%'] = MOD;

        // bitwise
        char_to_token_map['|'] = BIT_OR;
        char_to_token_map['&'] = BIT_AND;
        char_to_token_map['~'] = BIT_NOT;
        char_to_token_map['^'] = BIT_XOR;

        // boolean
        char_to_token_map['!'] = BOOL_NOT;

        // comparison
        char_to_token_map['>'] = GT;
        char_to_token_map['<'] = LT;

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

    if (tkn.sym == INDETERMINATE) {
        token_check_if_valid_literal_and_set_value(
            &tkn); // will appropriately set literal symbol and values, but will leave sym as
                   // INDETERMINATE if no pattern was matched
    }

    if (tkn.sym == INDETERMINATE) {
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
    return INDETERMINATE; // because strimap_viewn return NULL, it was thus not found and
                          // INDETERMINATE
}

void token_check_if_valid_literal_and_set_value(token_t* tkn) {
    if (!tkn || tkn->length == 0) {
        if (tkn) {
            tkn->sym = LEX_ERROR_EMPTY_TOKEN;
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
                tkn->sym = INDETERMINATE;
                return;
            }
        } else if (len == 3) { // simple char
            c = str[1];
        } else {
            tkn->sym = INDETERMINATE;
            return;
        }
        tkn->sym = CHAR_LIT;
        tkn->val.character = c;
        return;
    }

    // ~~~ STRING literal: "..." ~~~
    if (len >= 2 && str[0] == '"' && str[len - 1] == '"') {
        tkn->sym = STR_LIT;
        // value handling / escape sequences deferred
        return;
    }

    // ~~~ NUMERIC LITERALS ~~~
    // copy to temporary buffer and null-terminate
    char buf[64]; // may need to adjust very long numbers
    if (len >= sizeof(buf)) {
        tkn->sym = INDETERMINATE;
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
                tkn->sym = LONG_LIT;
                tkn->val.integral = integral_val;
            } else {
                tkn->sym = INT_LIT;
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
            tkn->sym = DOUB_LIT;
            tkn->val.floating = floating_val;
            return;
        }
    }

    tkn->sym = INDETERMINATE;
}

/**
 * check if a token is a valid variable/function name (a "symbol")
 */
void token_check_if_valid_symbol_and_set_sym(token_t* tkn) {
    // token should never have size zero
    if (tkn->start[0] >= '0' && tkn->start[0] <= '9') {
        tkn->sym = INDETERMINATE;
        return;
    }
    if (tkn->length <= 0) {
        tkn->sym = LEX_ERROR_EMPTY_TOKEN;
        return;
    }
    for (size_t i = 0; i < tkn->length; i++) {
        if (!isalnum(tkn->start[i]) && tkn->start[i] != '_') {
            tkn->sym = INDETERMINATE;
            return;
        }
    }
    tkn->sym = SYMBOL;
}
