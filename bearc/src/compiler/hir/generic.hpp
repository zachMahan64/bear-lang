//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_GENERICS_HPP
#define COMPILER_HIR_GENERICS_HPP

#include "compiler/hir/indexing.hpp"
#include "compiler/hir/variant_helpers.hpp"
#include <variant>
namespace hir {

using GenericArgValue = std::variant<ExecId, TypeId>;
struct GenericArg : NodeWithVariantValue<GenericArg> {
    using id_type = GenericArgId;
    using value_type = GenericArgValue;
    GenericArgValue value;
};

using GenericParamValue = std::variant<ExecId, DefId>;
struct GenericParam : NodeWithVariantValue<GenericParam> {
    using id_type = GenericArgId;
    using value_type = GenericParamValue;
    GenericParamValue value;
};

} // namespace hir

#endif // !COMPILER_HIR_GENERICS_HPP
