//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_INDEXING_H
#define COMPILER_HIR_INDEXING_H

#include "utils/string_view.h"
#include <stdint.h>

/// represents a null hir_id_t or its derivations, valued 0
#define HIR_ID_NONE 0

/// defines the underlying size of indices of vectors storing HIR nodes
typedef uint32_t hir_size_t;

/// abstract typedef id for indexing into vectors of HIR nodes
/// wrapped in structs for proper type checking
typedef hir_size_t hir_id_t;

/// abstract typedef indices for indexing into vectors of HIR node ids
/// useful for making contiguous slices of hir_id_* types
/// wrapped in structs for proper type checking
typedef hir_size_t hir_id_idx_t;

/// primary means of tracking interned strings in the hir
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

/// for addressing interned file names
typedef struct {
    hir_id_t val;
} hir_file_id_t;

/// for addressing named scopes
typedef struct {
    hir_id_t val;
} hir_scope_id_t;

/// for addressing anonymous scopes
typedef struct {
    hir_id_t val;
} hir_scope_anon_id_t;

/// for addressing exec nodes
typedef struct {
    hir_id_t val;
} hir_exec_id_t;

/// for addressing exec node ids
typedef struct {
    hir_id_idx_t val;
} hir_exec_id_idx_t;

typedef struct {
    hir_exec_id_idx_t start;
    hir_size_t len;
} hir_exec_id_slice_t;

/// for addressing definition nodes
typedef struct {
    hir_id_t val;
} hir_def_id_t;

/// for addressing contiguous slices of definition ids
typedef struct {
    hir_id_idx_t val;
} hir_def_id_idx_t;

/// for addressing type nodes
typedef struct {
    hir_id_t val;
} hir_type_id_t;

/// for addressing contiguous slices of type ids
typedef struct {
    hir_id_idx_t val;
} hir_type_id_idx_t;

/// for addressing generic parameter nodes
typedef struct {
    hir_id_t val;
} hir_generic_param_id_t;

/// for addressing contiguous slices of generic parameter ids
typedef struct {
    hir_id_idx_t val;
} hir_generic_param_id_idx_t;

/// for addressing generic argument nodes
typedef struct {
    hir_id_t val;
} hir_generic_arg_id_t;

/// for addressing contiguous slices of generic argument ids
typedef struct {
    hir_id_idx_t val;
} hir_generic_arg_id_idx_t;

/// for addressing parameter nodes
typedef struct {
    hir_id_t val;
} hir_param_id_t;

/// for addressing contiguous slices of parameter ids
typedef struct {
    hir_id_idx_t val;
} hir_param_id_idx_t;

#endif
