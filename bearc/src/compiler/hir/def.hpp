//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_DEF_H
#define COMPILER_HIR_DEF_H

#include "compiler/hir/indexing.hpp"
#include "compiler/hir/span.hpp"
#include <variant>

namespace hir {

// ------ struct impls -------
// TODO: finish impl structures

struct DefModule {
    ScopeId scope;
};

// ^^^^^^ struct impls ^^^^^^^^

/// main exec variant
using DefValue = std::variant<DefModule>;

/// main exec structure, corresponds to an hir_exec_id_t
struct Def {
    /*
        span: Span
        name: SymbolId
        parent: HirDefId?
        resolved: bool
        top_leveL_visited: bool // prevent and detect circular dependencies
        pub: bool
     */
    DefValue value;
    const Span span;
    const SymbolId name;
    /// value of HIR_ID_NONE (0) indicates no parent
    OptId<DefId> parent;
    const bool pub;
    bool top_level_visited;
    bool resolved;
};

} // namespace hir

#endif
