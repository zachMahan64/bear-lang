//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_IDENTIFER
#define COMPILER_HIR_IDENTIFER

#include "compiler/hir/span.hpp"

namespace hir {

/// represents an atom identifer in either type mention or variable mentions
typedef struct {
    IdSlice<SymbolId> symbols;
    Span span;
} hir_identifier_t;

} // namespace hir

#endif
