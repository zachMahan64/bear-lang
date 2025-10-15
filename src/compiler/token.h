// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef COMPILER_TOKEN_H
#define COMPILER_TOKEN_H

#include "containers/strimap.h"
#include <stddef.h>
#include <stdint.h>

#define TOKEN_MAX_KEYWORD_LEN 16
#define TOKEN_TOKEN_TO_STRING_MAP_SIZE 512
#define TOKEN_CHAR_TO_TOKEN_MAP_SIZE 256
#define TOKEN_STRING_TO_TOKEN_MAP_SIZE 256

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
    TYPE_GLUE = ':',

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
    KW_MT, // method, for implicit this ptr for data-associated behavior in structs
    KW_CT, // ctor
    KW_DT, // dtor
    // stdou
    KW_COUT, // example: cout <<- "Hello World";
    // types
    KW_BOX,   // example: box::int
    KW_BAG,   // example: bag::int
    KW_CONST, // example: const::box::int or box::const::int
    KW_REF,   // example: ref::int
    KW_INT,
    KW_LONG,
    KW_CHAR,
    KW_FLT,
    KW_DOUB,
    KW_STR,
    KW_BOOL,
    KW_VOID,
    KW_AUTO,
    KW_COMP,     // compile-time (like constexpr)
    KW_HIDDEN,   // like private, but with slightly different semantics because BearLang has no
                 // inheritance, so this is really just a hidden data member or function/method
    KW_TEMPLATE, // like a C++ template, with much more basic features (for), just like a smarter
                 // macro
    KW_ENUM,     // scoped/type-checked enums
    // memory location identifiers
    KW_STATIC,

    // comparison
    KW_IF,
    KW_ELSE,
    KW_ELIF,
    KW_WHILE,
    KW_FOR,
    KW_IN, // example: for (int a in myArr) {...}
    KW_RETURN,

    // structures (incorp after prodcedural is working)
    KW_THIS,
    KW_STRUCT,
    KW_NEW,

    // variable or function name
    SYMBOL,

    // built-in types
    CHAR_LIT,
    INT_LIT,
    FLT_LIT,
    STR_LIT,
    BOOL_LIT_FALSE,
    BOOL_LIT_TRUE,

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

    // --------------- compound assignment -----------------
    // arithmetic
    ASSIGN_PLUS_EQ,  // +=
    ASSIGN_MINUS_EQ, // -=
    ASSIGN_MULT_EQ,  // *=
    ASSIGN_DIV_EQ,   // /=
    ASSIGN_MOD_EQ,   // %=

    // bitwise
    ASSIGN_AND_EQ,  // &=
    ASSIGN_OR_EQ,   // |=
    ASSIGN_XOR_EQ,  // ^=
    ASSIGN_LSH_EQ,  // <<=
    ASSIGN_RSHL_EQ, // >>=
    ASSIGN_RSHA_EQ, // >>>=

    // add error states?
} token_type_e;

typedef union {
    char character;
    int64_t integral;
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
const char* get_char_to_token_map(void);
const strimap_t* get_string_to_token_strimap(void);
const char* const* get_token_to_string_map(void);
const char* get_always_one_char_to_token_map(void);
const char* get_first_char_in_multichar_operator_token_map(void);
// token struct functions
token_t token_build(const char* start, size_t length, src_loc_t* loc);

#endif // !COMPILER_TOKEN_H
