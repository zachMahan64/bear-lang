//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_ID_MAP_HPP
#define COMPILER_HIR_ID_MAP_HPP

#include "compiler/hir/indexing.hpp"
#include "utils/data_arena.hpp"
#include "utils/mapu32u32.h"
#include <stdint.h>
namespace hir {

// a hashmap optimized for Id storage, based on an internal arena that is not owned
template <hir::Id K, hir::Id V>
// should be the case given the constraints of hir::Id concept, but
// just be extra sure since this will otherwise implode
    requires(sizeof(K) == 4 && sizeof(V) == 4)
class IdMap {

    DataArena& arena;
    mapu32u32_t map;

  public:
    IdMap(const IdMap&) = delete;
    IdMap(IdMap&&) = default;
    IdMap& operator=(const IdMap&) = delete;
    IdMap& operator=(IdMap&&) = default;
    IdMap(DataArena& arena, size_t capacity)
        : arena(arena), map(mapu32u32_create_from_arena(capacity, arena.arena())) {}
    void insert(K key, V value) { mapu32u32_insert(&map, key.val(), value.val()); }
    bool remove(K key) { return mapu32u32_remove(&map, key.val()); }
    /// returns an optional id by value
    OptId<V> at(K key) const {
        uint32_t* value = mapu32u32_at(&map, key.val());
        if (value == nullptr) {
            return OptId<V>{};
        }
        return OptId<V>{*value};
    }
    bool contains(K key) { return static_cast<bool>(mapu32u32_at(&map, key.val())); }
};

} // namespace hir

#endif // !COMPILER_HIR_ID_MAP_HPP
