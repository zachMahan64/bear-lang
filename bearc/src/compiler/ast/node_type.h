#ifndef AST_NODE_TYPE_H
#define AST_NODE_TYPE_H

typedef enum {
    AST_EXPR_MODULE_NAME, // for module blocks

    // expr
    AST_EXPR_PRIMARY,
    AST_EXPR_GROUPING,
    AST_EXPR_BINARY,      // +, -, *, /, %, bitwise, comparison, boolean
    AST_EXPR_UNARY,       // unary -, !, ~, ++, --
    AST_EXPR_ASSIGN_EQ,   // = or
    AST_EXPR_ASSIGN_MOVE, // <- assignment
    AST_EXPR_FN_CALL,     // func(args...)

    // atoms
    AST_LITERAL,  // INT_LIT, FLOAT_LIT, CHAR_LIT, STRING_LIT
    AST_VARIABLE, // SCOPE ... + SYMBOL
    AST_SELF,     // TOK_SELF keyword

} ast_expr_e;

typedef enum {
    // blocks
    AST_STMT_BLOCK,  // {...} sequence of statements
    AST_STMT_MODULE, // AST_MODULE_NAME + AST_STMT_BLOCK
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
    AST_STMT_STRUCT_DEF, // TOK_STRUCT + fields
    AST_IMPORT,          // TOK_IMPORT + AST_MODULE_NAME
} ast_stmt_e;
#endif
