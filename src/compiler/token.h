#ifndef COMPILER_TOKEN_H
#define COMPILER_TOKEN_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    // file
    IMPORT,
    // keywords
    // namespace
    KW_SPACE, // space
    // function
    KW_FN,
    // stdou
    KW_COUT, // example: cout <<- "Hello World";
    // types
    KW_BOX,   // example: box::int
    KW_CONST, // example const::int
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
    RARROW,    // -> for return types
    SCOPE_RES, // ..
    TYPE_MOD,  // ::, for example, box:: (maybe different ones could be added later)

    // operators
    // assign
    ASSIGN_EQ,     // =
    ASSIGN_LARROW, // <-
    // stream
    STREAM, // <<-
    // arith
    PLUS,   // +
    MINUS,  // -
    MULT,   // *
    DIVIDE, // /
    MOD,    // %
    INC,    // ++
    DEC,    // --
    // bitwise
    BIT_OR,  // |
    BIT_AND, // &
    BIT_NOT, // ~
    BIT_XOR, // ^
    LSH,     // <<
    RSHL,    // >>
    RSHA,    // >>>
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
    size_t line, col;
} token_loc_t;

typedef struct {
    token_type_e sym;    // type
    const char* start;   // pointer into source
    size_t length;       // len in source
    token_value_u value; // get value using sym, only valid for literals
    token_loc_t loc;     // line & col, for error messages
} token_t;

#endif // !COMPILER_TOKEN_H
