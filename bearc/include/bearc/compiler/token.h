//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_TOKEN_H
#define COMPILER_TOKEN_H

#include "utils/strimap.h"
#include <stddef.h>
#include <stdint.h>

#define TOKEN_CHAR_TO_TOKEN_MAP_SIZE 256   // unsigned char max value
#define TOKEN_STRING_TO_TOKEN_MAP_SIZE 256 // abitrary, but large enough to reduce hashing conflicts

/// enum representing a type of token in the lexer
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

    // mark invocation
    TOK_HASH_INVOKE = '#',

    // arith
    TOK_PLUS = '+',   // +
    TOK_MINUS = '-',  // -
    TOK_STAR = '*',   // *
    TOK_DIVIDE = '/', // /
    TOK_MODULO = '%', // %

    // bitwise
    TOK_BAR = '|',     // |
    TOK_AMPER = '&',   // & ------> means bitwise and, also means reference
    TOK_BIT_NOT = '~', // ~
    TOK_BIT_XOR = '^', // ^

    // bool
    TOK_BOOL_NOT = '!', // !

    // comparison
    TOK_GT = '>', // will also be used as closing bracket in future generics
    TOK_LT = '<', // will also be used as opening bracket in future generics

    // file
    TOK_IMPORT = 256,
    // keywords
    // module
    TOK_MODULE, // mod
    // function
    TOK_FN,
    TOK_MT, // method, for implicit this ptr for data-associated behavior in structs
    TOK_DT, // dtor
    // stdio
    TOK_COUT, // example: cout <<- "Hello World";
    TOK_CIN,  // example: str myString <<- cin; // distinct from C++ which does std::cin >>
              // some_val;
    // built-in constructs
    TOK_OPT, // example: opt::i32
    TOK_RES, // example: res::u64
    TOK_BOX, // example: box::i32
    TOK_BAG, // example: bag::i32
    // integers
    TOK_I8,
    TOK_U8,
    TOK_I16,
    TOK_U16,
    TOK_I32,
    TOK_U32,
    TOK_I64,
    TOK_U64,
    TOK_USIZE,
    // char
    TOK_CHAR, // 32 bit unicode character
    // floating
    TOK_F32,
    TOK_F64,
    // special
    TOK_STR, // utf8 str
    TOK_BOOL,
    // more special
    TOK_VOID,     // no type
    TOK_VAR,      // like java var
    TOK_COMPT,    // compile-time (like constexpr)
    TOK_HID,      // like private, but with slightly different semantics because BearLang has no
                  // inheritance, so this is really just a hidden data member or function/method
    TOK_MUT,      // example: mut i32
    TOK_TEMPLATE, // like a C++ template, with much more basic features (for), just like a
                  // smarter macro
    TOK_ENUM,     // scoped/type-checked enums
    // memory location identifiers
    TOK_STATIC,

    // more operators
    TOK_SIZEOF,
    TOK_ALLIGNOF,
    TOK_AS,

    // marks
    TOK_MARK, // marks (like traits)
    TOK_REQUIRES,

    // comparison
    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_FOR,
    TOK_IN, // example: for (int a in myArr) {...}
    TOK_RETURN,

    // structures (incorp after prodcedural is working)
    TOK_SELF_ID,
    TOK_SELF_TYPE,
    TOK_STRUCT,

    // low level heap functions
    TOK_MALLOC,
    TOK_FREE,

    // variable or function name
    TOK_IDENTIFIER,

    // built-in types
    TOK_CHAR_LIT,
    TOK_INT_LIT,
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
    TOK_ASSIGN_MOVE, // <-
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

/// stores the value of literals in source code
typedef union {
    char character;
    int64_t integral;
    double floating;
} token_value_u;

/// represent a (line, col) index in a source file, zero-indexed
typedef struct {
    size_t line, col;
} src_loc_t;

/// represent a token in source code, generated by the lexer
typedef struct {
    /// NOT NULL-TERMINATED; non-owning view into source buffer
    const char* start;
    /// len in source
    size_t len;
    token_type_e type;
    /// get value using sym, only valid for numeric literals
    token_value_u val;
    /// line & col, for error messages
    src_loc_t loc;
} token_t;

typedef struct {
    token_t** start;
    size_t len;
} token_ptr_slice_t;

typedef struct {
    token_t* first;
    token_t* last;
} token_range_t;

/// returns a ptr to a char-to token map, used for mono-char tokens (e.g.: +), indexed by char
/// values!
const unsigned char* get_char_to_token_map(void);
/// returns a ptr to a strimap_t {const char*, token_type_e}
const strimap_t* get_string_to_token_strimap(void);
/// returns a ptr to a map for token_type_e -> const char*, indexed by token_type_e's!
const char* const* get_token_to_string_map(void);
/// returns a ptr to a map indicated whether a given char is always associated with a mono-char
/// token
const char* get_always_one_char_to_token_map(void);
/// returns a ptr to a map indicating whether a given char is the first char in a multichar operator
/// (e.g.: ==)
const char* get_first_char_in_multichar_operator_token_map(void);
/**
 * builds token according to a starting ptr and length into src as well as an src_loc_t that
 * indicates row/col number for debugging purposes. This function assumes that the lexer has already
 * correctly determined the string that needs to be tokenized.
 */
token_t token_build(const char* start, size_t length, src_loc_t* loc);

#endif // !COMPILER_TOKEN_H
