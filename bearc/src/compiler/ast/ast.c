#include "compiler/ast/ast.h"
#include "utils/arena.h"

void ast_destroy(ast_t* ast) { arena_destroy(&ast->arena); }

ast_t ast_create(const char* file_name) {
    ast_t ast;
    ast.file_name = file_name;
    ast.arena = arena_create(AST_ARENA_CHUNK_SIZE);
    return ast;
}
