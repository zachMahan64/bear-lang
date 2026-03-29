//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/context_database.hpp"

using namespace hir;

std::optional<hir::Def> ContextDatabase::query_definition(std::span<const char*> def_path) {
    llvm::SmallVector<SymbolId> sid_vec;
    for (const auto* const s : def_path) {
        sid_vec.push_back(ctx->symbol_id(s));
    }
    IdSlice<hir::Id<hir::Symbol>> sid_slice = ctx->freeze_id_vec(sid_vec);
    return std::nullopt;
}
