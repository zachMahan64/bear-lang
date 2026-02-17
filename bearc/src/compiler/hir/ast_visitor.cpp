//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/ast_visitor.hpp"
#include "compiler/ast/stmt.h"
#include "compiler/diagnostics/error_codes.h"
#include "compiler/hir/context.hpp"
#include "compiler/hir/def.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/hir/scope.hpp"
#include "compiler/token.h"
#include <variant>

namespace hir {

void AstVisitor::register_top_level_declarations() {
    // registers all the top level stmts of the file using the top level scope
    register_top_level_stmts(context.get_top_level_scope(),
                             context.ast(file).root()->stmt.file.stmts);
}

void AstVisitor::register_top_level_stmt(ScopeId scope, ast_stmt_t* stmt) {
    // handle visibility modifier
    bool pub = false;
    if (stmt->type == AST_STMT_VISIBILITY_MODIFIER) {
        pub = stmt->stmt.vis_modifier.modifier->type == TOK_PUB;
        // make stmt equal to inner
        stmt = stmt->stmt.vis_modifier.stmt;
    }

    bool compt = false;
    if (stmt->type == AST_STMT_COMPT_MODIFIER) {
        compt = true;
        // take inner
        stmt = stmt->stmt.compt_modifier.stmt;
    }

    bool statik = false;
    if (stmt->type == AST_STMT_STATIC_MODIFIER) {
        statik = true;
        // take inner
        stmt = stmt->stmt.static_modifier.stmt;
    }

    // handle module, first search for an existing module to insert into
    if (stmt->type == AST_STMT_MODULE) {
        SymbolId name = context.get_symbol_id_for_tkn(stmt->stmt.module.id);

        OptId<DefId> existing = Scope::look_up_namespace(context, scope, name).def_id;

        bool existing_module
            = existing.has_value()
              && std::holds_alternative<DefModule>(context.defs.cat(existing.as_id()).value);

        DefId def
            = existing_module
                  ? existing.as_id()
                  : context.register_top_level_def(
                        name, pub, Span(file, context.ast(file).buffer(), stmt->first, stmt->last),
                        stmt);
        ScopeId mod_scope = existing_module
                                ? get<DefModule>(context.defs.cat(existing.as_id()).value).scope
                                : context.make_named_scope();

        context.defs.at(def).set_value(DefModule{.scope = mod_scope, .name = name});
        context.scope(scope).insert_namespace(name, def);
        register_top_level_stmts(mod_scope, stmt->stmt.module.decls);
        return;
    }
    TopLevelInfo info = top_level_info_for(stmt);
    scope_kind kind = info.kind;
    token_t* name_tkn = info.name_tkn;
    std::optional<ast_slice_of_stmts_t> stmts = info.stmts;
    // if this wasn't named definition
    if (name_tkn == nullptr) {
        return;
    }

    // get symbol from token
    SymbolId name = context.get_symbol_id_for_tkn(name_tkn);

    // check for redefintion
    OptId<DefId> already_defined{};
    switch (kind) {
    case scope_kind::VARIABLE:
        already_defined = context.scope(scope).already_defines_variable(name);
        break;
    case scope_kind::TYPE:
        already_defined = context.scope(scope).already_defines_type(name);
        break;
    default:
        break;
    }

    // redefintion guard
    if (already_defined.has_value()) {
        context.ast(this->file).emplace_tokenwise_error(name_tkn, ERR_REDEFINITON);
        FileAst& orig_ast = context.ast(context.defs.cat(already_defined.as_id()).span.file_id);
        orig_ast.emplace_tokenwise_error(
            top_level_info_for(context.def_ast_nodes.at(already_defined.as_id())).name_tkn,
            NOTE_ORIGINAL_DEF_HERE);
        return;
    }

    // no issues, so register definition
    DefId def = context.register_top_level_def(
        name, pub, Span(file, context.ast(file).buffer(), stmt->first, stmt->last), stmt);

    switch (kind) {
    case scope_kind::VARIABLE:
        context.scope(scope).insert_variable(name, def);
        break;
    case scope_kind::TYPE:
        context.scope(scope).insert_type(name, def);
        break;
    default:
        break;
    }
}

void AstVisitor::register_top_level_stmts(ScopeId scope, ast_slice_of_stmts_t stmts) {
    for (size_t i = 0; i < stmts.len; i++) {
        register_top_level_stmt(scope, stmts.start[i]);
    }
}

TopLevelInfo AstVisitor::top_level_info_for(const ast_stmt_t* stmt) {
    scope_kind kind;
    token_t* name_tkn = nullptr;
    std::optional<ast_slice_of_stmts_t> stmts{};
    switch (stmt->type) {
    case AST_STMT_EXTERN_BLOCK: {
        // TODO
        break;
    }

    // internal scopes are deffered -----------------------------------
    case AST_STMT_STRUCT_DEF: {
        name_tkn = stmt->stmt.struct_decl.name;
        stmts = stmt->stmt.struct_decl.fields;
        kind = scope_kind::TYPE;
        break;
    }
    case AST_STMT_CONTRACT_DEF: {
        name_tkn = stmt->stmt.contract_decl.name;
        stmts = stmt->stmt.contract_decl.fields;
        kind = scope_kind::TYPE;
        break;
    }
    case AST_STMT_UNION_DEF: {
        name_tkn = stmt->stmt.union_decl.name;
        stmts = stmt->stmt.union_decl.fields;
        kind = scope_kind::TYPE;
        break;
    }
    case AST_STMT_VARIANT_DEF: {
        name_tkn = stmt->stmt.variant_decl.name;
        stmts = stmt->stmt.variant_decl.fields;
        kind = scope_kind::TYPE;
        break;
    }
        // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    case AST_STMT_VARIANT_FIELD_DECL: {
        name_tkn = stmt->stmt.variant_field_decl.name;
        kind = scope_kind::VARIABLE;
        break;
    }
    case AST_STMT_VAR_DECL: {
        name_tkn = stmt->stmt.var_decl.name;
        kind = scope_kind::VARIABLE;
        break;
    }
    case AST_STMT_VAR_INIT_DECL: {
        name_tkn = stmt->stmt.var_init_decl.name;
        kind = scope_kind::VARIABLE;
        break;
    }
    // TODO, needs to injected into the (struct,union,variant)'s scope
    // probably implement this outside of this method by adding a field to TopLevelInfo with
    // `parent_name_tkn`
    case AST_STMT_FN_DECL: {
        token_ptr_slice_t name_slice = stmt->stmt.fn_decl.name;
        // struct prefix name resolution deffered to later stages
        if (name_slice.len == 2) {
            name_tkn = name_slice.start[1];
        } else if (name_slice.len == 1) {
            name_tkn = name_slice.start[0];
        }
        kind = scope_kind::VARIABLE;
        break;
    }
    case AST_STMT_FN_PROTOTYPE: {
        // guranteed to be just one long
        name_tkn = stmt->stmt.fn_prototype.name.start[0];
        kind = scope_kind::VARIABLE;
        break;
    }
    case AST_STMT_DEFTYPE:
        name_tkn = stmt->stmt.deftype.alias_id;
        kind = scope_kind::TYPE;
        break;
    default:
        break;
    }
    return TopLevelInfo{.kind = kind, .name_tkn = name_tkn, .stmts = stmts};
}

} // namespace hir
