#ifndef AST_STATEMENTS_H
#define AST_STATEMENTS_H
#include "compiler/parser/parser.h"
#include "utils/vector.h"
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    // blocks
    AST_STMT_BLOCK,  // {...} sequence of statements
    AST_STMT_MODULE, // AST_MODULE_NAME + AST_STMT_BLOCK
    AST_STMT_FILE,

    // declarations
    AST_STMT_FN_DECL,       // fn + params + (body for definitions / null for declarations)
                            // + return type
    AST_STMT_MT_DECL,       // mt ^
    AST_STMT_DT_DECL,       // dt ^
    AST_STMT_VAR_DECL,      // type var;
    AST_STMT_VAR_EQ_DECL,   // type name = value
    AST_STMT_VAR_MOVE_DECL, // type name <- value;

    // control flow
    AST_STMT_IF,     // KW_IF + condition + then + else
    AST_STMT_ELSE,   // KW_ELSE (optional branch)
    AST_STMT_WHILE,  // KW_WHILE + condition + body
    AST_STMT_FOR,    // KW_FOR + init stmt + cond stmt + step expr + body stmt
    AST_STMT_FOR_IN, // KW_FOR + iterable + iterator + body stmt
    AST_STMT_RETURN, // KW_RETURN + expr

    // structs
    AST_STMT_STRUCT_DEF, // TOK_STRUCT + fields
    AST_STMT_IMPORT,     // TOK_IMPORT + AST_MODULE_NAME

    // marks
    AST_MARK_DECL, // mark Name { functions }
} ast_stmt_type_e;

// forward decls ~~~~~~~~~~~~~~~~~~
typedef struct ast_stmt ast_stmt_t;
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

/**
 * slice of statements
 */
typedef struct {
    ast_stmt_t* const start;
    const size_t len;
} ast_slice_of_stmts_t;

// stmt types ~~~~~~~~~~~~~~~~~~~~
typedef struct {
    ast_slice_of_stmts_t stmts;
} ast_stmt_block_t;

typedef struct {
    const char* file_name;
    ast_slice_of_stmts_t stmts;
} ast_stmt_file_t;

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// TODO, WIP
typedef union {
    ast_stmt_block_t block;
    ast_stmt_file_t file;
} ast_stmt_u;

typedef struct ast_stmt {
    ast_stmt_type_e type;
    ast_stmt_u stmt;
} ast_stmt_t;

#ifdef __cplusplus
} // extern "C"
#endif
#endif // !AST_STATEMENTS_H
