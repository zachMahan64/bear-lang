//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_GENERICS_HPP
#define COMPILER_HIR_GENERICS_HPP

#include "compiler/hir/indexing.hpp"
#include <variant>
namespace hir {

using GenericArgValue = std::variant<ExecId, TypeId>;
struct GenericArg {
    using id_type = GenericArgId;
    GenericArgValue value;
};

using GenericParamValue = std::variant<IdentifierId, ParamId>;
struct GenericParam {
    using id_type = GenericArgId;
    GenericParamValue value;
};

} // namespace hir

#endif // !COMPILER_HIR_GENERICS_HPP
