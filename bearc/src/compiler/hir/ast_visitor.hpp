//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_AST_VISITOR_HPP
#define COMPILER_HIR_AST_VISITOR_HPP

#include "compiler/ast/stmt_slice.h"
#include "compiler/hir/context.hpp"
#include "compiler/hir/indexing.hpp"
namespace hir {

struct TopLevelInfo {
    token_t* scope_prefix_tkn = nullptr;
    token_t* name_tkn = nullptr;
    std::optional<ast_slice_of_stmts_t> stmts;
    scope_kind kind;
};

class AstVisitor {
    Context& context;
    FileId file;
    void register_top_level_stmt(ScopeId scope, ast_stmt_t* stmt, OptId<DefId> parent,
                                 abi_lang abi);
    void register_top_level_stmts(ScopeId scope, ast_slice_of_stmts_t stmts, OptId<DefId> parent,
                                  abi_lang abi);
    static TopLevelInfo top_level_info_for(const ast_stmt_t* stmt);

  public:
    AstVisitor(Context& context, FileId file) : context(context), file(file) {}
    void register_top_level_declarations();
};

} // namespace hir

#endif
