#ifndef COMPILER_TOKEN_H
#define COMPILER_TOKEN_H

typedef enum {
    // variable or function name
    SYMBOL,
    // built-in types
    CHAR,
    INT,
    LONG,
    FLOAT,
    DOUBLE,
    PTR,

    // brackets
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACK,
    RBRACK,

    // operators
    // arith
    EQUALS,
    PLUS,
    MINUS,
    ASTERISK,
    DIVIDE,
    MOD,
    BIT_OR,
    BIT_AND,
    BIT_NOT,
    BIT_XOR,
    // bool
    BOOL_OR,
    BOOL_AND,
    BOOL_NOT,
    BOOL_XOR,

} token_type_e;

typedef struct {

} token_t;

#endif // !COMPILER_TOKEN_H
