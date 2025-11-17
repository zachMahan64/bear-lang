// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef COMPILER_TOKEN_H
#define COMPILER_TOKEN_H

#include "containers/strimap.h"
#include <stddef.h>
#include <stdint.h>

#define TOKEN_CHAR_TO_TOKEN_MAP_SIZE 256   // unsigned char max value
#define TOKEN_STRING_TO_TOKEN_MAP_SIZE 256 // abitrary, but large enough to reduce hashing conflicts

typedef enum token_type {
    TOK_NONE = 0,
    TOK_INDETERMINATE = 0,
    // mono-char tokens
    // delim
    // brackets, etc
    TOK_LPAREN = '(',
    TOK_RPAREN = ')', // ()
    TOK_LBRACE = '{',
    TOK_RBRACE = '}', // {}
    TOK_LBRACK = '[',
    TOK_RBRACK = ']', // []

    // punc
    TOK_SEMICOLON = ';',
    TOK_DOT = '.',
    TOK_COMMA = ',',
    TOK_COLON = ':',

    // assign
    TOK_ASSIGN_EQ = '=', // =

    // arith
    TOK_PLUS = '+',   // +
    TOK_MINUS = '-',  // -
    TOK_MULT = '*',   // *
    TOK_DIVIDE = '/', // /
    TOK_MOD = '%',    // %

    // bitwise
    TOK_BIT_OR = '|',  // |
    TOK_BIT_AND = '&', // &
    TOK_BIT_NOT = '~', // ~
    TOK_BIT_XOR = '^', // ^

    // bool
    TOK_BOOL_NOT = '!', // !

    // comparison
    TOK_GT = '>', // will also be used as closing bracket in future generics
    TOK_LT = '<', // will also be used as opening bracket in future generics

    // file
    TOK_KW_IMPORT = 256,
    // keywords
    // namespace
    TOK_KW_SPACE, // space
    // function
    TOK_KW_FN,
    TOK_KW_MT, // method, for implicit this ptr for data-associated behavior in structs
    TOK_KW_CT, // ctor
    TOK_KW_DT, // dtor
    // stdou
    TOK_KW_COUT, // example: cout <<- "Hello World";
    TOK_KW_CIN,  // example: str myString <<- cin; // distinct from C++ which does std::cin >>
                 // some_val;
    // types
    TOK_KW_BOX, // example: box int
    TOK_KW_BAG, // example: bag int
    TOK_KW_MUT, // example: mut int or box::const int
    TOK_KW_REF, // example: ref::int
    TOK_KW_INT,
    TOK_KW_UINT,
    TOK_KW_LONG,
    TOK_KW_ULONG,
    TOK_KW_CHAR,
    TOK_KW_FLT,
    TOK_KW_DOUB,
    TOK_KW_STR,
    TOK_KW_BOOL,
    TOK_KW_VOID,
    TOK_KW_AUTO,
    TOK_KW_COMP,     // compile-time (like constexpr)
    TOK_KW_HIDDEN,   // like private, but with slightly different semantics because BearLang has no
                     // inheritance, so this is really just a hidden data member or function/method
    TOK_KW_TEMPLATE, // like a C++ template, with much more basic features (for), just like a
                     // smarter macro
    TOK_KW_ENUM, // scoped/type-checked enums
    // memory location identifiers
    TOK_KW_STATIC,

    // comparison
    TOK_KW_IF,
    TOK_KW_ELSE,
    TOK_KW_ELIF,
    TOK_KW_WHILE,
    TOK_KW_FOR,
    TOK_KW_IN, // example: for (int a in myArr) {...}
    TOK_KW_RETURN,

    // structures (incorp after prodcedural is working)
    TOK_KW_THIS,
    TOK_KW_STRUCT,
    TOK_KW_NEW,

    // variable or function name
    TOK_SYMBOL,

    // built-in types
    TOK_CHAR_LIT,
    TOK_INT_LIT,
    TOK_LONG_LIT,
    TOK_DOUB_LIT, // currently float literals not suported, so we would need to downcast at comptime
    TOK_STR_LIT,
    TOK_BOOL_LIT_FALSE,
    TOK_BOOL_LIT_TRUE,

    // punc
    TOK_RARROW,    // -> for return types
    TOK_SCOPE_RES, // ..
    TOK_TYPE_MOD,  // :: , for example, box::int

    // operators
    // assign
    TOK_ASSIGN_LARROW, // <-
    // stream
    TOK_STREAM, // <<-
    // arith
    TOK_INC, // ++
    TOK_DEC, // --
    // bitwise
    TOK_LSH,  // <<
    TOK_RSHL, // >>
    TOK_RSHA, // >>>
    // bool
    TOK_BOOL_OR,  // ||
    TOK_BOOL_AND, // &&
    // comparison
    TOK_GE,
    TOK_LE,
    TOK_BOOL_EQ,
    TOK_NE,

    // --------------- compound assignment -----------------
    // arithmetic
    TOK_ASSIGN_PLUS_EQ,  // +=
    TOK_ASSIGN_MINUS_EQ, // -=
    TOK_ASSIGN_MULT_EQ,  // *=
    TOK_ASSIGN_DIV_EQ,   // /=
    TOK_ASSIGN_MOD_EQ,   // %=

    // bitwise
    TOK_ASSIGN_AND_EQ,  // &=
    TOK_ASSIGN_OR_EQ,   // |=
    TOK_ASSIGN_XOR_EQ,  // ^=
    TOK_ASSIGN_LSH_EQ,  // <<=
    TOK_ASSIGN_RSHL_EQ, // >>=
    TOK_ASSIGN_RSHA_EQ, // >>>=

    // EOF
    TOK_EOF,

    // error states (represent an error in lexing, should never happen and should not be triggered
    // by syntax errors)
    TOK_LEX_ERROR_EMPTY_TOKEN,

    // num token_type_e
    TOK__NUM,
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
const char* const* token_to_string_map(void);
const char* get_always_one_char_to_token_map(void);
const char* get_first_char_in_multichar_operator_token_map(void);
// token struct functions
token_t token_build(const char* start, size_t length, src_loc_t* loc);

#endif // !COMPILER_TOKEN_H
