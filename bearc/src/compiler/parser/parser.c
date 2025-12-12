#include "compiler/parser/parser.h"
#include "utils/arena.h"
#include <stddef.h>
parser_t parser_create(vector_t* tokens, arena_t* arena) {
    parser_t parser = {.tokens = *tokens, .pos = 0, .arena = *arena};
    return parser;
}
