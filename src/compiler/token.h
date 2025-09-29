#ifndef COMPILER_TOKEN_H
#define COMPILER_TOKEN_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    // keyword
    KW_INT,
    KW_CHAR,
    KW_FLOAT,
    KW_STRING,
    KW_IF,
    KW_ELSE,
    KW_ELIF,
    KW_WHILE,
    KW_FOR,
    KW_RETURN,

    SYMBOL, // variable or function name

    // built-in types
    CHAR_LIT,
    INT_LIT,
    FLOAT_LIT,
    STRING_LIT,

    // delim
    // brackets, etc
    LPAREN,
    RPAREN, // ()
    LBRACE,
    RBRACE, // {}
    LBRACK,
    RBRACK, // []

    // punc
    SEMICOLON,
    DOT,
    COMMA,
    RARROW,

    // operators
    // arith
    ASSIGN_EQ,     // =
    ASSIGN_LARROW, // <-
    PLUS,          // +
    MINUS,         // -
    ASTERISK,      // *
    DIVIDE,        // /
    MOD,           // %
    // bitwise
    BIT_OR,  // |
    BIT_AND, // &
    BIT_NOT, // ~
    BIT_XOR, // ^
    // bool
    BOOL_OR,  // ||
    BOOL_AND, // AND
    BOOL_NOT, // !
    // comparison
    GT,
    LT,
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
    token_type_e sym;
    const char* start;   // pointer into source
    size_t length;       // len in source
    token_value_u value; // get value using sym, only valid for literals
} token_t;

#endif // !COMPILER_TOKEN_H
