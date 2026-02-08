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

static constexpr size_t DEFAULT_SYMBOL_ARENA_CAP = 0x10000;
static constexpr size_t DEFAULT_ID_MAP_ARENA_CAP
    = 0x800; // increase if any other top level maps need to be made
static constexpr size_t DEFAULT_SYM_TO_FILE_ID_MAP_CAP = 0x80;
static constexpr size_t DEFAULT_SCOPE_VEC_CAP = 0x80;
static constexpr size_t DEFAULT_FILE_VEC_CAP = 0x80;
static constexpr size_t DEFAULT_FILE_AST_VEC_CAP = DEFAULT_FILE_VEC_CAP;
static constexpr size_t DEFAULT_FILE_ID_VEC_CAP = 0x200;
static constexpr size_t DEFAULT_SCOPE_ANON_VEC_CAP = 0x100;
static constexpr size_t DEFAULT_SYMBOL_VEC_CAP = 0x800;
static constexpr size_t DEFAULT_EXEC_VEC_CAP = 0x800;
static constexpr size_t DEFAULT_DEF_VEC_CAP = 0x800;
static constexpr size_t DEFAULT_TYPE_VEC_CAP = 0x400;
static constexpr size_t DEFAULT_GENERIC_PARAM_VEC_CAP = 0x80;
static constexpr size_t DEFAULT_GENERIC_ARG_VEC_CAP = 0x400;
// TODO: make a non default-ctor that actually calculates estimate capacities necessary (find these
// number empirically then apply here)
HirTables::HirTables()
    : symbol_arena{DEFAULT_SYMBOL_ARENA_CAP}, id_map_arena{DEFAULT_ID_MAP_ARENA_CAP},
      symbol_id_to_file_id_map{id_map_arena, DEFAULT_SYM_TO_FILE_ID_MAP_CAP},
      scopes{DEFAULT_SCOPE_VEC_CAP}, files{DEFAULT_FILE_VEC_CAP},
      file_asts{DEFAULT_FILE_AST_VEC_CAP}, scope_anons{DEFAULT_SCOPE_ANON_VEC_CAP},
      symbols{DEFAULT_SYMBOL_VEC_CAP}, execs{DEFAULT_EXEC_VEC_CAP}, defs{DEFAULT_DEF_VEC_CAP},
      file_ids{DEFAULT_FILE_ID_VEC_CAP}, importer_to_importees_vec{DEFAULT_FILE_VEC_CAP},
      importee_to_importers_vec{DEFAULT_FILE_VEC_CAP}, symbol_ids{DEFAULT_SYMBOL_VEC_CAP},
      exec_ids{DEFAULT_EXEC_VEC_CAP}, def_ids{DEFAULT_DEF_VEC_CAP},
      def_resolved{DEFAULT_DEF_VEC_CAP}, def_top_level_visited{DEFAULT_DEF_VEC_CAP},
      type_vec{DEFAULT_TYPE_VEC_CAP}, type_ids{DEFAULT_DEF_VEC_CAP},
      generic_param_ids{DEFAULT_GENERIC_PARAM_VEC_CAP},
      generic_params{DEFAULT_GENERIC_PARAM_VEC_CAP}, generic_arg_ids{DEFAULT_GENERIC_ARG_VEC_CAP},
      generic_args{DEFAULT_GENERIC_ARG_VEC_CAP} {}

} // namespace hir
