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

#ifdef __cplusplus
extern "C" {
#endif

void pretty_print_stmt(const ast_stmt_t* stmt);
void pretty_print_expr(const ast_expr_t* expr);
void pretty_printer_reset(void);

#ifdef __cplusplus
}
#endif

#endif
