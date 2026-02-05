//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_TYPE_HPP
#define COMPILER_HIR_TYPE_HPP

#include "compiler/hir/span.hpp"
#include <cstdint>
#include <variant>
namespace hir {

// ------ struct impls -------
// TODO: finish impl structures

enum class builtin_type : uint8_t {
    u8,
    i8,
    u16,
    i16,
    u32,
    i32,
    u64,
    i64,
    usize, // TODO finish
};

struct BuiltinType {
    builtin_type type;
};

// ^^^^^^ struct impls ^^^^^^^^

/// main exec union
using TypeValue = std::variant<BuiltinType>;

/// main exec structure, corresponds to an hir_exec_id_t
struct Type {
    TypeValue value;
    Span span;
};

} // namespace hir

#endif
