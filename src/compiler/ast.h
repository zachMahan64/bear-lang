// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef COMPILER_AST
#define COMPILER_AST

#include "containers/vector.h"
#include "token.h"

typedef enum {
    // Expressions
    AST_BINARY_OP, // +, -, *, /, %, bitwise, comparison, boolean
    AST_UNARY_OP,  // unary -, !, ~, ++, --
    AST_LITERAL,   // INT_LIT, FLOAT_LIT, CHAR_LIT, STRING_LIT
    AST_VARIABLE,  // SYMBOL
    AST_ASSIGN,    // = or <- assignment

    // Function/Call
    AST_FUNCTION_DEF,  // KW_FN + body + params + return type
    AST_FUNCTION_CALL, // func(args...)

    // Control Flow
    AST_IF,     // KW_IF + condition + then + else
    AST_ELIF,   // KW_ELIF (optional chain)
    AST_ELSE,   // KW_ELSE (optional branch)
    AST_WHILE,  // KW_WHILE + condition + body
    AST_FOR,    // KW_FOR + init + condition + step + body
    AST_RETURN, // KW_RETURN + value

    // Structural / Blocks
    AST_BLOCK,      // {...} sequence of statements
    AST_STRUCT_DEF, // KW_STRUCT + fields + optional impl
    AST_IMPL,       // KW_IMPL for method implementations
    AST_IMPORT,     // IMPORT statements
    AST_NAMESPACE,  // KW_SPACE for namespace blocks
    AST_THIS,       // KW_THIS keyword

    // Other / Punctuation / Scope
    AST_TYPE_MOD,  // :: for type modifiers
    AST_SCOPE_RES, // .. scope resolution

    // Unknown / fallback
    AST_UNKNOWN
} ast_node_type_e;

typedef struct ast_node_s {
    ast_node_type_e type;
    token_t* token;    // must be non-NULL for literals/operators/variables
    vector_t children; // dynamic list of child nodes
} ast_node_t;

#endif // !COMPILER_AST
