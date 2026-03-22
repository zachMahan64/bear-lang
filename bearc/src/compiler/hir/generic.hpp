//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_GENERICS_HPP
#define COMPILER_HIR_GENERICS_HPP

#include "compiler/hir/indexing.hpp"
#include "compiler/hir/variant_helpers.hpp"
#include "utils/data_arena.hpp"
#include <variant>
namespace hir {

using GenericArgValue = std::variant<ExecId, TypeId>;
struct GenericArg : NodeWithVariantValue<GenericArg> {
    using id_type = GenericArgId;
    using value_type = GenericArgValue;
    GenericArgValue value;
};

using GenericParamValue = std::variant<ExecId, DefId>;
struct GenericParam : NodeWithVariantValue<GenericParam> {
    using id_type = GenericArgId;
    using value_type = GenericParamValue;
    GenericParamValue value;
};

class Context;

// TODO implement
class CanonicalGenericArgsTable {
    struct Entry {
        GenericArgIdSliceId key_id;
        CanonicalGenericArgsId val;
        size_t hash;
        Entry* next;
        Entry(GenericArgIdSliceId key_id, CanonicalGenericArgsId val, size_t hash, Entry* next)
            : key_id(key_id), val(val), hash(hash), next(next) {}
    };
    static constexpr size_t DEFAULT_CAP = 128;
    static constexpr double LOAD_FACTOR = 1.25;
    // internally, this consider mut recursively:
    Context& context;
    DataArena& arena;

    Entry** buckets;
    size_t count;
    size_t capacity;

    void rehash(size_t new_capacity);
    bool same_structure(GenericArgIdSliceId tid1, GenericArgIdSliceId tid2) const;
    size_t hash(TypeId type) const;
    static size_t index(size_t hash, size_t cap);
    static void put_new_head_on_chain(Entry** chain, Entry* new_entry);
    // only use after at returns none to avoid duplicate inserts
    void insert(TypeId tid, CanonicalGenericArgsId cid);

  public:
    CanonicalGenericArgsTable(Context& context, DataArena& arena, HirSize capacity)
        : context{context}, arena{arena}, capacity{capacity} {}
    OptId<CanonicalGenericArgsId> at(GenericArgIdSliceId tid) const;
    [[nodiscard]] CanonicalGenericArgsId canonical(GenericArgIdSliceId tid);
};

} // namespace hir

#endif // !COMPILER_HIR_GENERICS_HPP
