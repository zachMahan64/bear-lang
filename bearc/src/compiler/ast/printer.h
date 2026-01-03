//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_AST_PRINTER_H
#define COMPILER_AST_PRINTER_H

#include "compiler/ast/stmt.h"
void print_stmt(ast_stmt_t* stmt);
void print_expr(ast_expr_t* expr);
void printer_reset(void);

#endif
