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
    using id_type = DefId;
    /// underlying structure
    DefValue value;
    /// span in src
    const Span span;
    /// id corresponding to the interned identifier
    const SymbolId name;
    /// parent's definition, if any
    OptId<DefId> parent;
    /// indicates pub (true) or hid (false) visibility
    const bool pub;
};

} // namespace hir

#endif
