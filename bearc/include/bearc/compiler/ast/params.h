//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_AST_PARAMS
#define COMPILER_AST_PARAMS
#include "compiler/token.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ast_type ast_type_t;

typedef struct {
    ast_type_t* type;
    token_t* name;
    bool valid;
    token_t* first;
    token_t* last;
} ast_param_t;

typedef struct {
    ast_param_t** start;
    size_t len;
} ast_slice_of_params_t;

#ifdef __cplusplus
} // extern "C"
#endif
#endif // !COMPILER_AST_PARAMS
