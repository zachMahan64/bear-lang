//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/builder.hpp"
#include "cli/args.h"
#include "compiler/hir/tables.hpp"

size_t hir_build(const bearc_args_t* args) {

    return 0; // todo return # errors
}
namespace hir {
namespace builder {

/// creates an HIR database from a file_name
HirTables build_from_file(const char* file_name) {
    FileAst root_ast{file_name};
    return HirTables{}; // TODO
}

} // namespace builder
} // namespace hir
