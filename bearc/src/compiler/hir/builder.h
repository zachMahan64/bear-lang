//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_BUILDER_H
#define COMPILER_HIR_BUILDER_H

#include "cli/args.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t hir_build(const bearc_args_t* args);
#ifdef __cplusplus
}
#endif
#endif
