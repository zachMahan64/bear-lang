#include "token.h"
#include "containers/strimap.h"
#include <stdbool.h>

const int* get_char_to_token_map(void) {
    static bool initialized = false;
    static int char_to_token_map[128]; // ASCII lookup
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

const strimap_t* get_string_to_token_strimap(void) {
    static bool initialized = false;
    static strimap_t map;

    if (!initialized) {
        map = strimap_create(128); // make large to reduce hash conflicts

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
