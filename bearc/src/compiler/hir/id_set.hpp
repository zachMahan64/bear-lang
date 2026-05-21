//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/id_hash_map.hpp"
#include "compiler/hir/indexing.hpp"
#include "utils/data_arena.hpp"

namespace hir {

template <typename T> class IdSet {
  public:
    IdSet(DataArena& arena, HirSize capacity) : map{arena, capacity} {}
    [[nodiscard]] HirSize size() { return size_; }
    /// returns true if the set already contained the value
    bool insert(T elt) {
        const bool already_has = map.contains(elt);
        map.insert(elt, DUMMY);
        if (!already_has) {
            ++size_;
        }
        return already_has;
    }
    [[nodiscard]] bool contains(T elt) { return map.contains(elt); }
    // TODO

  private:
    hir::IdHashMap<T, HirSize> map;
    HirSize size_;
    static constexpr HirSize DUMMY = 0;
};

} // namespace hir
