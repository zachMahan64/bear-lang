//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_IDENTIFER_HPP
#define COMPILER_HIR_IDENTIFER_HPP

#include "compiler/hir/indexing.hpp"
#include "compiler/hir/span.hpp"

namespace hir {

/// represents an atom identifer in either type mention or variable mentions
struct Identifier {
    const IdSlice<SymbolId> symbols;
    const Span span;
    OptId<DefId> resolved_to_def;
    bool is_resolved() const noexcept { return resolved_to_def.has_value(); }
    void resolve(DefId def_id) noexcept { this->resolved_to_def.set(def_id); }
};

} // namespace hir

#endif
