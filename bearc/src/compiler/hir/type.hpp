//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_TYPE_HPP
#define COMPILER_HIR_TYPE_HPP

#include "compiler/hir/indexing.hpp"
#include "compiler/hir/span.hpp"
#include <cstdint>
#include <variant>
namespace hir {

// ------ struct impls -------

enum class builtin_type : uint8_t {
    u8,
    i8,
    u16,
    i16,
    u32,
    i32,
    u64,
    i64,
    usize,
    charr,
    f32,
    f64,
    voidd,
    str,

};

struct TypeBuiltin {
    builtin_type type;
};

struct TypeStructure {
    SymbolId name;
};

struct TypeGenericStructure {
    SymbolId name;
    IdSlice<GenericArgId> slice;
};

struct TypeArr {
    TypeId inner;
    ExecId compt_size_expr;
    size_t canonical_size;
};

struct TypeSlice {
    TypeId inner;
};

struct TypeRef {
    TypeId inner;
};

struct TypePtr {
    TypeId inner;
};

struct TypeFnPtr {
    IdSlice<TypeId> param_types;
    TypeId return_type;
};

struct TypeVariadic {
    TypeId inner;
};

// ^^^^^^ struct impls ^^^^^^^^

/// main exec union
using TypeValue = std::variant<TypeBuiltin, TypeStructure, TypeGenericStructure, TypeArr, TypeSlice,
                               TypeRef, TypePtr, TypeFnPtr, TypeVariadic>;

/// main exec structure, corresponds to an hir_exec_id_t
struct Type {
    using id_type = TypeId;
    TypeValue value;
    Span span;
    bool mut;
};

} // namespace hir

#endif
