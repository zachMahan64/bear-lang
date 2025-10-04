#ifndef COMPILER_TOKEN_H
#define COMPILER_TOKEN_H

#include "containers/strimap.h"
#include <stddef.h>
#include <stdint.h>

#define TOKEN_MAX_KEYWORD_LEN 16
#define TOKEN_TOKEN_TO_STRING_MAP_SIZE 512
#define TOKEN_CHAR_TO_TOKEN_MAP_SIZE 128
#define TOKEN_STRING_TO_TOKEN_MAP_SIZE 128

typedef enum token_type {
    INDETERMINATE = 0,
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
    KW_FLT,
    KW_STR,

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
    FLT_LIT,
    STR_LIT,

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
    // add error states?
} token_type_e;

typedef union {
    char character;
    int64_t integer;
    double floating;
} token_value_u;

typedef struct {
    size_t line, col;
} src_loc_t;

typedef struct {
    const char* start; // NOT NULL-TERMINATED; non-owning view into source buffer
    size_t length;     // len in source
    token_type_e sym;  // type
    token_value_u val; // get value using sym, only valid for numeric literals
    src_loc_t loc;     // line & col, for error messages
} token_t;

// token map functions
const int* get_char_to_token_map(void);
const strimap_t* get_string_to_token_strimap(void);
const char* const* get_token_to_string_map(void);
// token struct functions
token_t token_build(const char* start, size_t length, src_loc_t* loc);

#endif // !COMPILER_TOKEN_H
