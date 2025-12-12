// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef COMPILER_PARSER_H
#define COMPILER_PARSER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "compiler/ast/ast.h"
#include "compiler/diagnostics/error_list.h"
#include "compiler/token.h"
#include "utils/vector.h"
#include <stdint.h>

// ctor that builds up an ast from a specified vector of tokens
ast_t parser_build_ast_from_file(const char* file_name, vector_t token_vec,
                                 compiler_error_list_t* error_list);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // !COMPILER_PARSER_H
