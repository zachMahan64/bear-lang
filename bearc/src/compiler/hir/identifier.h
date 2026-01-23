//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_IDENTIFIER
#define COMPILER_HIR_IDENTIFIER

#include "compiler/span/span.h"
#include <stdint.h>

typedef struct {
    uint32_t val;
} hir_identifier_id_t;

typedef struct {
    span_t span;
} hir_identifier_t;

#endif
