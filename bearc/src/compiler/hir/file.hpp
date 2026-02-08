//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_FILE_HPP
#define COMPILER_HIR_FILE_HPP
#include "compiler/ast/ast.h"
#include "compiler/ast/stmt_slice.h"
#include "compiler/hir/indexing.hpp"
#include "utils/file_io.h"
#include <cstdint>

namespace hir {

enum file_load_state : uint8_t {
    FILE_LOAD_STATE_UNVISITING = 0,
    FILE_LOAD_STATE_IN_PROGRESS,
    FILE_LOAD_STATE_DONE
};

struct File {
    using id_type = FileId;
    SymbolId path;
    FileAstId ast_id;
    file_load_state load_state;
};

class FileAst {
    using id_type = FileAstId;
    br_ast_t ast;

  public:
    const src_buffer& buffer() const noexcept;
    const compiler_error_list_t& error_list() const noexcept;
    const ast_stmt_t* root() const noexcept;
    FileAst(const char* file_name);
    ~FileAst();
};

} // namespace hir

#endif
