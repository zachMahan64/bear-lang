//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_EXEC_HPP
#define COMPILER_HIR_EXEC_HPP

#include "compiler/hir/indexing.hpp"
#include "compiler/hir/span.hpp"
#include <variant>

namespace hir {

// ------ struct impls -------
// TODO: finish impl structures

typedef struct {
    ScopeAnonId scope;
    IdSlice<ExecId> execs;
} ExecBlock;

// ^^^^^^ struct impls ^^^^^^^^

/// main exec union
using ExecValue = std::variant<ExecBlock>;

/// main exec structure, corresponds to an hir_exec_id_t
typedef struct {
    ExecValue exec;
    Span span;
    const bool compt;
} Exec;

} // namespace hir

#endif
