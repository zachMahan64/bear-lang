#ifndef AST_AST_H
#define AST_AST_H
#ifdef __cplusplus
extern "C" {
#endif

#include "utils/arena.h"
#define AST_ARENA_CHUNK_SIZE 0x10000

typedef struct {
    // TODO
    const char* file_name;
    arena_t arena;
} ast_t;

// ctor for ast_t
ast_t ast_create(const char* file_name);
// dtor for ast_t
void ast_destroy(ast_t* ast);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
