//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/ast_visitor.hpp"
#include "compiler/ast/stmt.h"
#include "compiler/hir/context.hpp"
#include "compiler/hir/def.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/token.h"

namespace hir {

void AstVisitor::register_top_level_declarations() {
    // registers all the top level stmts of the file using the top level scope
    register_top_level_stmts(context.get_top_level_scope(),
                             context.ast(file).root()->stmt.file.stmts);
}

/// TODO, handle namespace insertion and also handle redefintions
void AstVisitor::register_top_level_stmt(ScopeId scope, ast_stmt_t* stmt) {
    // handle visibility modifier
    bool pub = false;
    if (stmt->type == AST_STMT_VISIBILITY_MODIFIER) {
        pub = stmt->stmt.vis_modifier.modifier->type == TOK_PUB;
        // make stmt equal to inner
        stmt = stmt->stmt.vis_modifier.stmt;
    }

    // handle module, TODO fix this logic to first search for an existing module to insert into
    if (stmt->type == AST_STMT_MODULE) {
        SymbolId name = context.get_symbol_id_for_tkn(stmt->stmt.module.id);
        DefId def = context.register_top_level_def(
            name, pub, Span(file, context.ast(file).buffer(), stmt->first, stmt->last), stmt);
        ScopeId mod_scope = context.make_named_scope();
        context.defs.at(def).set_value(DefModule{.scope = mod_scope, .name = name});
        context.scopes.at(scope).insert_namespace(name, def);
        register_top_level_stmts(mod_scope, stmt->stmt.module.decls);
        return;
    }

    token_t* tkn;
    switch (stmt->type) {
    case AST_STMT_EXTERN_BLOCK:
    case AST_STMT_VAR_DECL:
    case AST_STMT_VAR_INIT_DECL:
    case AST_STMT_VISIBILITY_MODIFIER:
    case AST_STMT_COMPT_MODIFIER:
    case AST_STMT_STATIC_MODIFIER:
    case AST_STMT_STRUCT_DEF:
    case AST_STMT_CONTRACT_DEF:
    case AST_STMT_UNION_DEF:
    case AST_STMT_VARIANT_DEF:
    case AST_STMT_VARIANT_FIELD_DECL:
    case AST_STMT_FN_DECL:
    case AST_STMT_FN_PROTOTYPE:
    case AST_STMT_DEFTYPE:
        break;
    default:
        return;
    }
    SymbolId name = context.get_symbol_id_for_tkn(tkn);
    DefId def = context.register_top_level_def(
        name, pub, Span(file, context.ast(file).buffer(), stmt->first, stmt->last), stmt);
}

void AstVisitor::register_top_level_stmts(ScopeId scope, ast_slice_of_stmts_t stmts) {
    for (size_t i = 0; i < stmts.len; i++) {
        register_top_level_stmt(scope, stmts.start[i]);
    }
}

} // namespace hir
