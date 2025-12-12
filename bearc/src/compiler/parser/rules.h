#ifndef COMPILER_PARSER_RULES

#include "compiler/ast/expr.h"
#include "compiler/parser/parser.h"
#include <stdint.h>

/// defines the associativity of an operator
typedef enum {
    LEFT_TO_RIGHT,
    RIGHT_TO_LEFT,
} associativity_e;

associativity_e associativity_of(uint32_t precedence);

// find the precendence an operator based on token type
uint32_t precendence_of_operator(token_type_e type);

#endif // !COMPILER_PARSER_RULES
