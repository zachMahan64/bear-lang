//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/ast_visitor.hpp"
#include "compiler/ast/printer.h"
#include "compiler/ast/stmt.h"
#include "compiler/hir/context.hpp"
#include "compiler/hir/def.hpp"
#include "compiler/hir/diagnostic.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/hir/scope.hpp"
#include "compiler/hir/type.hpp"
#include "compiler/token.h"
#include "llvm/ADT/SmallVector.h"
#include <variant>

namespace hir {

bool is_lower(const token_t* s);
bool is_capital(const token_t* s);
std::optional<abi_lang> abi_for_extern_stmt(const ast_stmt_t* stmt);

void FileAstVisitor::register_top_level_declarations() {
    // registers all the top level stmts of the file using the top level scope
    register_top_level_stmts(context.get_or_make_root_scope(),
                             context.ast(file).root()->stmt.file.stmts, OptId<DefId>{},
                             abi_lang::native);
}

OptId<DefId> FileAstVisitor::register_top_level_stmt(ScopeId scope, ast_stmt_t* stmt,
                                                     OptId<DefId> parent, abi_lang abi) {
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
        SymbolId name = context.symbol_id_for_identifier_tkn(name_tkn);

        // look up a LOCAL namespace since we don't want to traverse parents for already defined
        // namespaces, because we want to allow nested namespaces. If we didn't do this, we might
        // have this situation:
        // mod foo { i32 BAR = 42; mod bar { i32 BAR = 42; } }
        // mod foey { mod foo { i32 BAR = 42; } }
        // where mod foo is found in the parent of mod foey and then BAR is added to the outer
        // definition of foo!
        //
        // this would be bad.
        OptId<DefId> existing = Scope::look_up_local_namespace(context, scope, name).as_id();

        bool existing_module
            = existing.has_value()
              && std::holds_alternative<DefModule>(context.defs.cat(existing.as_id()).value);

        DefId mod_def = existing_module
                            ? existing.as_id()
                            : context.register_top_level_def(
                                  name, pub, compt, /*not generic*/ false, statik,
                                  Span(file, context.ast(file).buffer(), stmt->first, stmt->last),
                                  stmt, parent);
        ScopeId mod_scope = existing_module
                                ? get<DefModule>(context.defs.cat(existing.as_id()).value).scope
                                : context.make_named_scope(scope);
        // warn capitalized_mod if the mod is new and capitalized
        if (!existing_module && is_capital(name_tkn)) {
            context.emplace_diagnostic(Span(file, context.ast(file).buffer(), name_tkn),
                                       diag_code::capitalized_mod, diag_type::warning);
        }
        context.defs.at(mod_def).set_value(DefModule{.scope = mod_scope, .name = name});
        context.scope(scope).insert_namespace(name, mod_def);
        register_top_level_stmts(mod_scope, stmt->stmt.module.decls, mod_def,
                                 abi); // pass in this module def as parent
        return OptId<DefId>{};
    }
    // handle extern block
    if (stmt->type == AST_STMT_EXTERN_BLOCK) {
        auto maybe_abi = abi_for_extern_stmt(stmt);
        // ensure valid specified abi
        if (!maybe_abi.has_value()) {
            auto did = context.emplace_diagnostic(
                Span(file, context.ast(file).buffer(), stmt->stmt.extern_block.extern_language),
                diag_code::invalid_extern_lang, diag_type::error,
                DiagnosticSymbolAfterMessage{
                    context.symbol_id(stmt->stmt.extern_block.extern_language)},
                DiagnosticNoOtherInfo{});
            auto didn = context.emplace_diagnostic(
                Span(file, context.ast(file).buffer(), stmt->stmt.extern_block.extern_language),
                diag_code::declare_this_as, diag_type::help,
                DiagnosticSymbolAfterMessage{context.symbol_id("extern C")},
                DiagnosticNoOtherInfo{});

        } else {
            enum abi_lang abi = maybe_abi.value();
            register_top_level_stmts(scope, stmt->stmt.extern_block.decls, parent, abi);
        }
        return OptId<DefId>{};
    }
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    const TopLevelInfo info = top_level_info_for(stmt);
    const scope_kind kind = info.kind;
    const token_t* name_tkn = info.name_tkn;
    const std::optional<ast_slice_of_stmts_t> stmts = info.stmts;
    const bool is_generic = info.is_generic;

    // if this wasn't named definition, then RETURN so we don't try to make a new hir::Def
    if (name_tkn == nullptr) {
        return OptId<DefId>{};
    }

    // hanlde scope prefix for Foo..bar() functions
    token_t* prefix = info.scope_prefix_tkn;
    if (prefix) {
        auto r = Scope::look_up_type(context, scope,
                                     context.symbol_id_for_identifier_tkn(info.scope_prefix_tkn));
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
    SymbolId name = context.symbol_id_for_identifier_tkn(name_tkn);

    // check for redefintion
    OptId<DefId> already_defined{};
    switch (kind) {
    case scope_kind::variable:
        already_defined = context.scope(scope).already_defines_variable(name);
        break;
    case scope_kind::type:
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
                                             diag_code::previous_def_here, diag_type::note);
        context.set_next_diagnostic(d1, d2);
        return OptId<DefId>{};
    }

    // no issues, so register definition
    DefId def = context.register_top_level_def(
        name, pub, compt, statik, is_generic,
        Span(file, context.ast(file).buffer(), stmt->first, stmt->last), stmt, parent);
    // register into a scope
    if (!info.do_not_insert_in_scope) {
        switch (kind) {
        case scope_kind::variable:
            context.scope(scope).insert_variable(name, def);
            break;
        case scope_kind::type: {
            context.scope(scope).insert_type(name, def);
            // delay resolution (don't clutter scope tree with unspecialized/dead definitions)
            if (is_generic) {
                break;
            }
            // if the type (namely a variant field decl) doesn't have statements then the scope
            // needn't be large
            const bool is_small_scope = !stmts.has_value();
            ScopeId types_scope = (is_small_scope) ? context.make_small_named_scope(scope)
                                                   : context.make_named_scope(scope);
            context.def_to_scope_for_types.insert(def, types_scope);
            // warn on lowercase structure definition
            if (is_lower(name_tkn)) {
                context.emplace_diagnostic(Span(file, context.ast(file).buffer(), name_tkn),
                                           diag_code::lowercase_structure, diag_type::warning);
            }
            // try to parse fields
            if (info.stmts.has_value()) {
                register_top_level_stmts_registering_ordered_members(def, types_scope,
                                                                     info.stmts.value(), def, abi);
            }
            break;
        }

        default:
            break;
        }
    }
    // return the DefId since this is an orderable definition
    if (info.is_orderable_var) {
        return def;
    }
    return OptId<DefId>{};
}

void FileAstVisitor::register_top_level_stmts(ScopeId scope, ast_slice_of_stmts_t stmts,
                                              OptId<DefId> parent, abi_lang abi) {
    for (size_t i = 0; i < stmts.len; i++) {
        register_top_level_stmt(scope, stmts.start[i], parent, abi);
    }
}
void FileAstVisitor::register_top_level_stmts_registering_ordered_members(
    DefId parent_def, ScopeId scope, ast_slice_of_stmts_t stmts, OptId<DefId> parent,
    abi_lang abi) {
    llvm::SmallVector<DefId> def_vec{};
    for (size_t i = 0; i < stmts.len; i++) {
        OptId<DefId> maybe_def = register_top_level_stmt(scope, stmts.start[i], parent, abi);
        if (maybe_def.has_value()) {
            def_vec.emplace_back(maybe_def.as_id());
        }
    }

    // handle no members
    if (def_vec.empty()) {
        const ast_stmt_t* st = context.def_ast_nodes.cat(parent_def);
        const ast_stmt_type_e statement_type = st->type;
        const Span span{file, context.ast(file).buffer(), top_level_info_for(st).name_tkn};
        diag_code code = diag_code::empty_struct;
        switch (statement_type) {
        case AST_STMT_STRUCT_DEF:
            code = diag_code::empty_struct;
            break;
        case AST_STMT_UNION_DEF:
            code = diag_code::empty_union;
            break;
        case AST_STMT_VARIANT_DEF:
            code = diag_code::empty_variant;
            break;
        case AST_STMT_CONTRACT_DEF:
            // this is actually expected to be empty since it's just method decls
            return; // we're done here
        default:
            pretty_print_stmt(st);
            assert(false && "tried to register ordered defs for an invalid type");
            break;
        }
        context.emplace_diagnostic(span, code, diag_type::error);
    } else {
        // register the defs
        context.register_ordered_defs(parent_def, def_vec);
    }
}
TopLevelInfo FileAstVisitor::top_level_info_for(const ast_stmt_t* stmt) {
    scope_kind kind = scope_kind::variable;
    token_t* scope_prefix_tkn = nullptr;
    token_t* name_tkn = nullptr;
    std::optional<ast_slice_of_stmts_t> stmts{};
    bool is_orderable_field = false;
    bool is_generic = false;
    bool do_not_insert_in_scope = false;
    switch (stmt->type) {
    // internal scopes are deffered -----------------------------------
    case AST_STMT_STRUCT_DEF: {
        name_tkn = stmt->stmt.struct_decl.name;
        stmts = stmt->stmt.struct_decl.fields;
        kind = scope_kind::type;
        is_generic = stmt->stmt.struct_decl.is_generic;
        break;
    }
    case AST_STMT_CONTRACT_DEF: {
        name_tkn = stmt->stmt.contract_decl.name;
        stmts = stmt->stmt.contract_decl.fields;
        kind = scope_kind::type;
        break;
    }
    case AST_STMT_UNION_DEF: {
        name_tkn = stmt->stmt.union_decl.name;
        stmts = stmt->stmt.union_decl.fields;
        kind = scope_kind::type;
        break;
    }
    case AST_STMT_VARIANT_DEF: {
        name_tkn = stmt->stmt.variant_decl.name;
        stmts = stmt->stmt.variant_decl.fields;
        kind = scope_kind::type;
        is_generic = stmt->stmt.variant_decl.is_generic;
        break;
    }
        // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    case AST_STMT_VARIANT_FIELD_DECL: {
        name_tkn = stmt->stmt.variant_field_decl.name;
        kind = scope_kind::type;
        is_orderable_field = true;
        break;
    }
    case AST_STMT_VAR_DECL: {
        name_tkn = stmt->stmt.var_decl.name;
        kind = scope_kind::variable;
        is_orderable_field = true;
        break;
    }
    case AST_STMT_VAR_INIT_DECL: {
        name_tkn = stmt->stmt.var_init_decl.name;
        kind = scope_kind::variable;
        is_orderable_field = true;
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
        kind = scope_kind::variable;
        is_generic = stmt->stmt.fn_decl.is_generic;
        break;
    }
    case AST_STMT_FN_PROTOTYPE: {
        // guranteed to be just one long
        name_tkn = stmt->stmt.fn_prototype.name.start[0];
        kind = scope_kind::variable;
        is_generic = stmt->stmt.fn_prototype.is_generic;
        break;
    }
    case AST_STMT_DEFTYPE:
        name_tkn = stmt->stmt.deftype.alias_id;
        kind = scope_kind::type;
        do_not_insert_in_scope = true; // since we want this tkn to forward directly to another
                                       // type, so this will have to be handled in resolution later
        break;
    default:
        break;
    }
    return TopLevelInfo{.scope_prefix_tkn = scope_prefix_tkn,
                        .name_tkn = name_tkn,
                        .stmts = stmts,
                        .kind = kind,
                        .is_orderable_var = is_orderable_field,
                        .is_generic = is_generic,
                        .do_not_insert_in_scope = do_not_insert_in_scope};
}

std::optional<const token_t*> FileAstVisitor::name_of_ast_decl(const ast_stmt_t* stmt) {
    const token_t* tkn_ptr = top_level_info_for(stmt).name_tkn;
    return (tkn_ptr) ? tkn_ptr : std::optional<const token_t*>{};
}

// some helpers

bool is_lower(const token_t* s) { return !is_capital(s); }
bool is_capital(const token_t* s) { return s->start[0] >= 'A' && s->start[0] <= 'Z'; }
std::optional<abi_lang> abi_for_extern_stmt(const ast_stmt_t* stmt) {
    if (stmt->stmt.extern_block.extern_language == nullptr) {
        return abi_lang::native;
    }
    return (stmt->stmt.extern_block.extern_language->len != 0
            && stmt->stmt.extern_block.extern_language->start[0] == 'C')
               ? abi_lang::c
               : std::optional<abi_lang>{};
}

} // namespace hir
