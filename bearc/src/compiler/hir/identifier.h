//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_IDENTIFER
#define COMPILER_HIR_IDENTIFER

#include "compiler/hir/indexing.h"
#include "compiler/span/span.h"

/// represents an atom identifer in either type mention or variable mentions
typedef struct {
    hir_symbol_id_idx_t symbol_ids_start;
    hir_size_t symbols_ids_len;
    span_t span;
} hir_identifier_t;

#endif
