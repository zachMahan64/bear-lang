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

struct DefModule {
    ScopeId scope;
    SymbolId name;
};

enum class abi : uint8_t {
    native = 0,
    c,
};

struct DefFunction {
    SymbolId name;
    IdSlice<DefId> params;
    OptId<TypeId> return_type;
    ExecId body;
    /// if this function was derived from an original generic function
    OptId<DefId> original;
    // indicates if this is extern C compatible
    hir::abi lang = abi::native;
};

struct DefGenericFunction {
    IdSlice<GenericParamId> generic_params;
    SymbolId name;
    // potentially problematic
    DefId underlying_def;
};

struct DefFunctionPrototype {
    IdSlice<DefId> params;
    OptId<TypeId> return_type;
    hir::abi lang = abi::native;
};

struct DefVariable {
    TypeId type;
    SymbolId name;
    OptId<ExecId> compt_value;
    bool moved;
};

struct DefStruct {
    ScopeId scope;
    SymbolId name;
    /// data members/fields, not functions
    IdSlice<DefId> ordered_members;
    IdSlice<DefId> contracts;
    /// if this struct was derived from an original generic struct
    OptId<DefId> orginal;
};

struct DefGenericStruct {
    IdSlice<GenericParamId> generic_params;
    SymbolId name;
    // potentially problematic
    DefId underlying_def;
};

struct DefVariant {
    ScopeId scope;
    SymbolId name;
    /// if this variant was derived from an original generic variant
    OptId<DefId> orginal;
};

struct DefGenericVariant {
    IdSlice<GenericParamId> generic_params;
    SymbolId name;
    // potentially problematic
    DefId underlying_def;
};

struct DefVariantField {
    SymbolId name;
    /// pointing to DefVariables which containg type and name info
    IdSlice<DefId> members;
};

struct DefUnion {
    ScopeId scope;
    SymbolId name;
};

struct DefContract {
    ScopeId scope;
    SymbolId name;
};

struct DefDefType {
    SymbolId name;
    TypeId type;
};

struct DefUnevaluated {};

// ^^^^^^ struct impls ^^^^^^^^

/// main exec variant
using DefValue
    = std::variant<DefModule, DefFunction, DefGenericFunction, DefFunctionPrototype, DefVariable,
                   DefStruct, DefGenericStruct, DefVariant, DefGenericVariant, DefVariantField,
                   DefUnion, DefContract, DefDefType, DefUnevaluated>;

enum class storage : uint8_t { normal, statik, compt };

/// main exec structure, corresponds to an hir_exec_id_t
struct Def {
    /// represents the resolution state corresponding to an hir::DefId
    enum class resol_state : uint8_t {
        unvisited = 0,
        top_level_visited,
        // can help catch circular defintions
        in_progress,
        resolved,
    };

    /// represents the mention state corresponding to an hir::DefId
    enum class mention_state : uint8_t {
        unmentioned,
        mentioned,
        modified,
    };
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
    const bool pub = false;
    /// indicates compt (compile-time)
    const bool compt = false;
    /// indicates static (storage duration)
    const bool statik = false;
    Def(DefValue& value, SymbolId name, bool pub, bool compt, bool statik, Span span, DefId parent)
        : value{value}, span{span}, name{name}, pub{pub}, parent{parent}, compt{compt},
          statik{statik} {}
    Def(DefValue& value, SymbolId name, bool pub, Span span)
        : value{value}, span{span}, name{name}, pub{pub} {}
    Def(SymbolId name, bool pub, Span span)
        : span{span}, name{name}, pub{pub}, value{DefUnevaluated{}} {}
    Def(SymbolId name, bool pub, Span span, OptId<DefId> parent)
        : span{span}, name{name}, pub{pub}, value{DefUnevaluated{}}, parent{parent} {}
    void set_value(DefValue value) { this->value = value; }
};

} // namespace hir

#endif
