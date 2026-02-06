//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/tables.hpp"
#include <stddef.h>
namespace hir {

static constexpr size_t DEFAULT_SYMBOL_ARENA_SIZE = 0x10000;
static constexpr size_t DEFAULT_ID_MAP_ARENA_SIZE
    = 0x800; // increase if any other top level maps need to be made
static constexpr size_t DEFAULT_SYM_TO_FILE_ID_MAP_SIZE = 0x80;

HirTables::HirTables()
    : symbol_arena(DEFAULT_SYMBOL_ARENA_SIZE), id_map_arena(DEFAULT_ID_MAP_ARENA_SIZE),
      symbol_id_to_file_id_map(id_map_arena, DEFAULT_SYM_TO_FILE_ID_MAP_SIZE) {}

} // namespace hir
