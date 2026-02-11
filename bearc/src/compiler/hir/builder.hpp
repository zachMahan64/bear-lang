//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_BUILDER_H
#define COMPILER_HIR_BUILDER_H

#include "cli/args.h"
#include "compiler/hir/tables.hpp"
namespace hir {
namespace builder {

/// creates an HIR database from a file_name
HirTables from_file(const char* file_name, const bearc_args_t* args);

} // namespace builder
} // namespace hir

#endif // COMPILER_HIR_BUILDER_H
