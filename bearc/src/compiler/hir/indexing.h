//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_INDEXING
#define COMPILER_HIR_INDEXING

#include "utils/string_view.h"
#include <stdint.h>

/// defines the underlying size of indices of vectors storing HIR nodes
typedef uint32_t hir_size_t;

/// abstract typedef id for indexing into vectors of HIR nodes
/// *** wrap in a struct for proper type checking
typedef hir_size_t hir_id_t;

/// abstract typedef indices for indexing into vectors of HIR node ids
/// useful for making contiguous slices of hir_id_* types
/// *** wrap in a struct for proper type checking
typedef hir_size_t hir_id_idx_t;

/// primary means of tracking interns strings in the hir
typedef struct {
    hir_id_t val;
} hir_symbol_id_t;

/// for addressing symbol id's
typedef struct {
    hir_id_idx_t val;
} hir_symbol_id_idx_t;

/// to be stored in a hir_symbol_id_t -> hir_symbol_t table
typedef struct {
    /// string view in src file of first mention
    /// used purely as a string for reversing symbol ids back into strings
    string_view_t string_view;
} hir_symbol_t;

/// for indexing interned file names
typedef struct {
    hir_id_t val;
} hir_file_id_t;

#endif
