#include "token.h"
#include "containers/strimap.h"
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// returns a view into statically allocated map of char -> token_type_e
const int* get_char_to_token_map(void) {
    static bool initialized = false;
    static int char_to_token_map[TOKEN_CHAR_TO_TOKEN_MAP_SIZE]; // ASCII lookup
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

        // file
        strimap_insert(&map, "import", IMPORT);

        // keywords
        strimap_insert(&map, "space", KW_SPACE);
        strimap_insert(&map, "fn", KW_FN);
        strimap_insert(&map, "cout", KW_COUT);
        strimap_insert(&map, "box", KW_BOX);
        strimap_insert(&map, "const", KW_CONST);
        strimap_insert(&map, "ref", KW_REF);
        strimap_insert(&map, "int", KW_INT);
        strimap_insert(&map, "char", KW_CHAR);
        strimap_insert(&map, "flt", KW_FLT);
        strimap_insert(&map, "str", KW_STR);

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
        strimap_insert(&map, "impl", KW_IMPL);

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
        strimap_insert(&map, "==", EQ);
        strimap_insert(&map, "!=", NE);

        initialized = true;
    }

    return &map;
}

// returns a view into statically allocated map of token_type_e -> string
const char* const* get_token_to_string_map(void) {
    static bool initialized = false;
    static const char* map[TOKEN_TOKEN_TO_STRING_MAP_SIZE] = {0};

    if (!initialized) {
        map[INDETERMINATE] = "INDETERMINATE";

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
        map[KW_COUT] = "cout";
        map[KW_BOX] = "box";
        map[KW_CONST] = "const";
        map[KW_REF] = "ref";
        map[KW_INT] = "int";
        map[KW_CHAR] = "char";
        map[KW_FLT] = "flt";
        map[KW_STR] = "str";

        map[KW_IF] = "if";
        map[KW_ELSE] = "else";
        map[KW_ELIF] = "elif";
        map[KW_WHILE] = "while";
        map[KW_FOR] = "for";
        map[KW_RETURN] = "return";

        // structures
        map[KW_THIS] = "this";
        map[KW_STRUCT] = "struct";
        map[KW_IMPL] = "impl";

        // variable / literal types
        map[SYMBOL] = "symbol";
        map[CHAR_LIT] = "char_lit";
        map[INT_LIT] = "int_lit";
        map[FLT_LIT] = "float_lit";
        map[STR_LIT] = "string_lit";

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
        map[EQ] = "==";
        map[NE] = "!=";

        initialized = true;
    }
    return map;
}

// token struct functions
token_type_e token_determine_token_type_for_fixed_symbols(const char* start, size_t length);
void token_check_if_valid_literal_and_set_value(token_t* tkn);

// Builds token according to a starting ptr and length into src as well as an src_loc_t that
// indicates row/col number for debugging purposes. This function assumes that the lexer has already
// correct determined the string that needs to be tokenized.
token_t build_token(const char* start, size_t length, src_loc_t* loc) {
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

    // TODO
    return tkn;
}

// helper that looks up tokens for fixed symbols (i.e keywords and operators), impls basic logic
// using different look ups for optimization
token_type_e token_determine_token_type_for_fixed_symbols(const char* start, size_t length) {
    const int* char_to_token_map = get_char_to_token_map();
    if (length == 1) {
        return char_to_token_map[start[0]]; // return token indexed at the mono-char literal, any
                                            // invalid entry will be automatically '\0' since the
                                            // map was statically initialized and token
                                            // INDETERMINATE = 0
    }
    const strimap_t* string_to_token_map = get_string_to_token_strimap();
    token_type_e* tkn_type_ptr_from_multi_char_map =
        (token_type_e*)strimap_viewn(string_to_token_map, start, length);
    if (tkn_type_ptr_from_multi_char_map != NULL) {
        return *tkn_type_ptr_from_multi_char_map; // derefence and return because it was valid and
                                                  // thus found
    }
    return INDETERMINATE; // because strimap_viewn return NULL, it was thus not found and
                          // INDETERMINATE
}

void token_check_if_valid_literal_and_set_value(token_t* tkn) {
    if (!tkn || tkn->length == 0) {
        if (tkn) {
            tkn->sym = INDETERMINATE;
        }
        return;
    }

    const char* str = tkn->start;
    size_t len = tkn->length;

    // ~~~ CHAR literal: 'a' or escapes like '\n' ~~~
    if (str[0] == '\'' && str[len - 1] == '\'' && len >= 3) {
        char c;
        if (str[1] == '\\') { // escape sequence
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
            case '\"':
                c = '\"';
                break;
            case '0':
                c = '\0';
                break;
            default:
                tkn->sym = INDETERMINATE;
                return; // unsupported escape
            }
        } else if (len == 3) {
            c = str[1];
        } else {
            tkn->sym = INDETERMINATE;
            return; // multi-char literal invalid
        }
        tkn->sym = CHAR_LIT;
        tkn->val.character = c;
        return;
    }

    // ~~~ STRING literal: "..." ~~~
    if (str[0] == '"' && str[len - 1] == '"') {
        tkn->sym = STR_LIT;
        // val unused; escapes resolved later
        return;
    }

    // ~~~ INTEGER literal (decimal, hex 0x, octal 0, binary 0b) ~~~
    errno = 0;
    char* endptr = NULL;
    long long int_val = strtoll(str, &endptr, 0); // base 0 auto-detects 0x / 0 prefix
    if (endptr != str && errno == 0) {
        while (isspace((unsigned char)*endptr)) {
            endptr++;
        }
        if (*endptr == '\0') {
            tkn->sym = INT_LIT;
            tkn->val.integer = int_val;
            return;
        }
    }

    // ~~~ FLOAT literal (scientific notation handled) ~~~
    errno = 0;
    endptr = NULL;
    double float_val = strtod(str, &endptr);
    if (endptr != str && errno == 0) {
        while (isspace((unsigned char)*endptr)) {
            endptr++;
        }
        if (*endptr == '\0') {
            tkn->sym = FLT_LIT;
            tkn->val.floating = float_val;
            return;
        }
    }

    tkn->sym = INDETERMINATE; // fallback, ensure this
}
