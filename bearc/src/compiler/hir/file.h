//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_FILE_HPP
#define COMPILER_HIR_FILE_HPP
#include "compiler/hir/indexing.hpp"
#include <cstdint>

namespace hir {

enum file_load_state : uint8_t {
    FILE_LOAD_STATE_UNVISITING = 0,
    FILE_LOAD_STATE_IN_PROGRESS,
    FILE_LOAD_STATE_DONE
};

struct File {
    SymbolId path;
    file_load_state load_state;
    AstId ast_id;
};

} // namespace hir

#endif
