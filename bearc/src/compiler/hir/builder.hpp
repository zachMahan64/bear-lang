//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_BUILDER_H
#define COMPILER_HIR_BUILDER_H

#include "compiler/hir/tables.hpp"
#include <string_view>
namespace hir {
namespace builder {

/// creates an HIR database from a file_name
HirTables build_from_file(std::string_view file_name); // TODO

} // namespace builder
} // namespace hir

#endif // COMPILER_HIR_BUILDER_H
