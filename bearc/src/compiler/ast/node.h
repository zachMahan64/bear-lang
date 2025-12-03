// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef COMPILER_AST
#define COMPILER_AST

#ifdef __cplusplus
extern "C" {
#endif

#include "compiler/token.h"
#include <stddef.h>

typedef enum {
    // Unknown / fallback
    AST_NONE = 0,
    // Blocks
    AST_MODULE,      // AST_MODULE_NAME + AST_STMT_BLOCK
    AST_MODULE_NAME, // for module blocks
    AST_STMT_BLOCK,  // {...} sequence of statements
    // expr
    AST_EXPR_BINARY,      // +, -, *, /, %, bitwise, comparison, boolean
    AST_EXPR_UNARY,       // unary -, !, ~, ++, --
    AST_EXPR_ASSIGN_EQ,   // = or
    AST_EXPR_ASSIGN_MOVE, // <- assignment
    AST_EXPR_FN_CALL,     // func(args...)
    // atoms
    AST_LITERAL,  // INT_LIT, FLOAT_LIT, CHAR_LIT, STRING_LIT
    AST_VARIABLE, // SCOPE ... + SYMBOL

    // statements
    AST_STMT_FN_DEC, // KW_FN + params + (body for definitions / null for declarations)
                     // + return type
    AST_STMT_MT_DEC,
    AST_STMT_DT_DEC,

    // Control Flow
    AST_STMT_IF,     // KW_IF + condition + then + else
    AST_STMT_ELIF,   // KW_ELIF (optional chain)
    AST_STMT_ELSE,   // KW_ELSE (optional branch)
    AST_STMT_WHILE,  // KW_WHILE + condition + body
    AST_STMT_FOR,    // KW_FOR + init stmt + cond stmt + step expr + body stmt
    AST_STMT_FOR_IN, // KW_FOR + iterable + iterator + body stmt
    AST_STMT_RETURN, // KW_RETURN + expr

    // structs
    AST_STRUCT_DEF, // TOK_STRUCT + fields
    AST_IMPORT,     // TOK_IMPORT + AST_MODULE_NAME
    AST_SELF,       // TOK_SELF keyword

} ast_node_type_e;

typedef struct ast_node {
    ast_node_type_e type;
    token_t* token; // must be non-NULL for literals/operators/variables
    size_t child_count;
    struct ast_node* children[]; // var len array of child nodes
} ast_node_t;

// add a child at a specified index inside the specified node.
void ast_node_add_child(ast_node_t* node, size_t idx, ast_node_t* child);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !COMPILER_AST
