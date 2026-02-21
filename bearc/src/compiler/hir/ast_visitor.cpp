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
#include "compiler/hir/diagnostic.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/hir/scope.hpp"
#include "compiler/token.h"
#include <variant>

namespace hir {

bool is_lower(const token_t* s);
bool is_capital(const token_t* s);
std::optional<abi_lang> abi_for_extern_stmt(const ast_stmt_t* stmt);

void AstVisitor::register_top_level_declarations() {
    // registers all the top level stmts of the file using the top level scope
    register_top_level_stmts(context.get_top_level_scope(),
                             context.ast(file).root()->stmt.file.stmts, OptId<DefId>{},
                             abi_lang::native);
}

void AstVisitor::register_top_level_stmt(ScopeId scope, ast_stmt_t* stmt, OptId<DefId> parent,
                                         abi_lang abi) {
    // handle prefix wrappers --------
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
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    // special cases (modules and extern blocks)
    // handle module, first search for an existing module to insert into
    if (stmt->type == AST_STMT_MODULE) {
        token_t* name_tkn = stmt->stmt.module.id;
        SymbolId name = context.get_symbol_id_for_tkn(name_tkn);

        OptId<DefId> existing = Scope::look_up_namespace(context, scope, name).def_id;

        bool existing_module
            = existing.has_value()
              && std::holds_alternative<DefModule>(context.defs.cat(existing.as_id()).value);

        DefId mod_def = existing_module
                            ? existing.as_id()
                            : context.register_top_level_def(
                                  name, pub, compt, statik,
                                  Span(file, context.ast(file).buffer(), stmt->first, stmt->last),
                                  stmt, parent);
        ScopeId mod_scope = existing_module
                                ? get<DefModule>(context.defs.cat(existing.as_id()).value).scope
                                : context.make_named_scope();
        // warn capitalized_mod if the mod is new and capitalized
        if (!existing_module && is_capital(name_tkn)) {
            context.emplace_diagnostic(Span(file, context.ast(file).buffer(), name_tkn),
                                       diag_code::capitalized_mod, diag_type::warning);
        }
        context.defs.at(mod_def).set_value(DefModule{.scope = mod_scope, .name = name});
        context.scope(scope).insert_namespace(name, mod_def);
        register_top_level_stmts(mod_scope, stmt->stmt.module.decls, mod_def,
                                 abi); // pass in this module def as parent
        return;
    }
    // handle extern block
    if (stmt->type == AST_STMT_EXTERN_BLOCK) {
        auto maybe_abi = abi_for_extern_stmt(stmt);
        // ensure valid specified abi
        if (!maybe_abi.has_value()) {
            context.emplace_diagnostic(
                Span(file, context.ast(file).buffer(), stmt->stmt.extern_block.extern_language),
                diag_code::invalid_extern_lang, diag_type::error);

        } else {
            enum abi_lang abi = maybe_abi.value();
            register_top_level_stmts(scope, stmt->stmt.extern_block.decls, parent, abi);
        }
        return;
    }
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    TopLevelInfo info = top_level_info_for(stmt);
    scope_kind kind = info.kind;
    token_t* name_tkn = info.name_tkn;
    std::optional<ast_slice_of_stmts_t> stmts = info.stmts;

    // if this wasn't named definition, then RETURN so we don't try to make a new hir::Def
    if (name_tkn == nullptr) {
        return;
    }

    // hanlde scope prefix for Foo..bar() functions
    token_t* prefix = info.scope_prefix_tkn;
    if (prefix) {
        auto r = Scope::look_up_type(context, scope,
                                     context.get_symbol_id_for_tkn(info.scope_prefix_tkn));
        bool no_struct = false;
        if (r.status == scope_look_up_status::okay) {
            if (auto s = context.def_to_scope_for_types.at(r.def_id); s.has_value()) {
                scope = s.as_id();
            } else {
                no_struct = true;
            }
        } else {
            no_struct = true;
        }

        if (no_struct) {
            context.emplace_diagnostic(Span(file, context.ast(file).buffer(), prefix),
                                       diag_code::no_matching_struct_for_method, diag_type::error);
        }
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
        // do diagnostics for the redefinition
        auto d1 = context.emplace_diagnostic(Span(file, context.ast(file).buffer(), name_tkn),
                                             diag_code::redefinition, diag_type::error);
        auto orig_file = context.defs.cat(already_defined.as_id()).span.file_id;
        auto* t = top_level_info_for(context.def_ast_nodes.at(already_defined.as_id())).name_tkn;
        auto d2 = context.emplace_diagnostic(Span(orig_file, context.ast(orig_file).buffer(), t),
                                             diag_code::original_def_here, diag_type::note);
        context.set_next_diagnostic(d1, d2);
        return;
    }

    // no issues, so register definition
    DefId def = context.register_top_level_def(
        name, pub, compt, statik, Span(file, context.ast(file).buffer(), stmt->first, stmt->last),
        stmt, parent);

    switch (kind) {
    case scope_kind::VARIABLE:
        context.scope(scope).insert_variable(name, def);
        break;
    case scope_kind::TYPE:
        context.scope(scope).insert_type(name, def);
        context.def_to_scope_for_types.insert(
            def, context.make_named_scope()); // warn on lowercase structure definition
        if (is_lower(name_tkn)) {
            context.emplace_diagnostic(Span(file, context.ast(file).buffer(), name_tkn),
                                       diag_code::lowercase_structure, diag_type::warning);
        }
        break;
    default:
        break;
    }
}

void AstVisitor::register_top_level_stmts(ScopeId scope, ast_slice_of_stmts_t stmts,
                                          OptId<DefId> parent, abi_lang abi) {
    for (size_t i = 0; i < stmts.len; i++) {
        register_top_level_stmt(scope, stmts.start[i], parent, abi);
    }
}

TopLevelInfo AstVisitor::top_level_info_for(const ast_stmt_t* stmt) {
    scope_kind kind = scope_kind::VARIABLE;
    token_t* scope_prefix_tkn = nullptr;
    token_t* name_tkn = nullptr;
    std::optional<ast_slice_of_stmts_t> stmts{};
    switch (stmt->type) {
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
    case AST_STMT_FN_DECL: {
        token_ptr_slice_t name_slice = stmt->stmt.fn_decl.name;
        // struct prefix name resolution deffered to later stages
        if (name_slice.len == 2) {
            scope_prefix_tkn = name_slice.start[0];
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
    return TopLevelInfo{
        .scope_prefix_tkn = scope_prefix_tkn,
        .name_tkn = name_tkn,
        .stmts = stmts,
        .kind = kind,
    };
}

bool is_lower(const token_t* s) { return !is_capital(s); }
bool is_capital(const token_t* s) { return s->start[0] >= 'A' && s->start[0] <= 'Z'; }
std::optional<abi_lang> abi_for_extern_stmt(const ast_stmt_t* stmt) {
    return (stmt->stmt.extern_block.extern_language->len != 0
            && stmt->stmt.extern_block.extern_language->start[0] == 'C')
               ? abi_lang::c
               : std::optional<abi_lang>{};
}

} // namespace hir
