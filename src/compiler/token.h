#ifndef COMPILER_TOKEN_H
#define COMPILER_TOKEN_H

#include "containers/strimap.h"
#include <stddef.h>
#include <stdint.h>

typedef enum token_type {
    // mono-char tokens
    // delim
    // brackets, etc
    LPAREN = '(',
    RPAREN = ')', // ()
    LBRACE = '{',
    RBRACE = '}', // {}
    LBRACK = '[',
    RBRACK = ']', // []

    // punc
    SEMICOLON = ';',
    DOT = '.',
    COMMA = ',',

    // assign
    ASSIGN_EQ = '=', // =

    // arith
    PLUS = '+',   // +
    MINUS = '-',  // -
    MULT = '*',   // *
    DIVIDE = '/', // /
    MOD = '%',    // %

    // bitwise
    BIT_OR = '|',  // |
    BIT_AND = '&', // &
    BIT_NOT = '~', // ~
    BIT_XOR = '^', // ^

    // bool
    BOOL_NOT = '!', // !

    // comparison
    GT = '>',
    LT = '<',

    // file
    IMPORT = 256,
    // keywords
    // namespace
    KW_SPACE, // space
    // function
    KW_FN,
    // stdou
    KW_COUT, // example: cout <<- "Hello World";
    // types
    KW_BOX,   // example: box::int
    KW_CONST, // example const int
    KW_REF,   // ref
    KW_INT,
    KW_CHAR,
    KW_FLOAT,
    KW_STRING,

    // comparison
    KW_IF,
    KW_ELSE,
    KW_ELIF,
    KW_WHILE,
    KW_FOR,
    KW_RETURN,

    // structures (incorp after prodcedural is working)
    KW_THIS,
    KW_STRUCT,
    KW_IMPL,

    // variable or function name
    SYMBOL,

    // built-in types
    CHAR_LIT,
    INT_LIT,
    FLOAT_LIT,
    STRING_LIT,

    // punc
    RARROW,    // -> for return types
    SCOPE_RES, // ..
    TYPE_MOD,  // ::, for example, box:: (maybe different ones could be added later)

    // operators
    // assign
    ASSIGN_LARROW, // <-
    // stream
    STREAM, // <<-
    // arith
    INC, // ++
    DEC, // --
    // bitwise
    LSH,  // <<
    RSHL, // >>
    RSHA, // >>>
    // bool
    BOOL_OR,  // ||
    BOOL_AND, // &&
    // comparison
    GE,
    LE,
    EQ,
    NE,

} token_type_e;

typedef union {
    char character;
    int64_t integer;
    double floating;
} token_value_u;

typedef struct {
    size_t line, col;
} token_loc_t;

typedef struct {
    token_type_e sym;    // type
    const char* start;   // pointer into source
    size_t length;       // len in source
    token_value_u value; // get value using sym, only valid for literals
    token_loc_t loc;     // line & col, for error messages
} token_t;

int* get_char_to_token_map(void);
strimap_t* get_string_to_token_strimap(void);

#endif // !COMPILER_TOKEN_H
