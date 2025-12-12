#ifndef COMPILER_PARSER_STMT
#define COMPILER_PARSER_STMT

#include "compiler/ast/stmt.h"
#include "compiler/diagnostics/error_list.h"

/// ctor that builds up an ast from a specified vector of tokens
ast_stmt_file_t parser_file(const char* file_name, vector_t token_vec,
                            compiler_error_list_t* error_list);

#endif
