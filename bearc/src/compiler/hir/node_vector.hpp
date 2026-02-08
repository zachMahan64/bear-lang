//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_NODE_VECTOR_HPP
#define COMPILER_HIR_NODE_VECTOR_HPP

#include "compiler/hir/indexing.hpp"
#include <cassert>
#include <utility>
#include <vector>
namespace hir {
template <typename T>
concept Node = hir::Id<typename T::id_type>;

template <hir::Id I, typename V> class IdVecMap {
    std::vector<V> vec;
    static constexpr HirId OFFSET = 1;

  public:
    explicit IdVecMap(HirSize capacity) : vec{} { vec.reserve(capacity + OFFSET); }
    void reserve(HirSize size) { vec.reserve(size); }
    [[nodiscard]] V& at(I id) {
        assert(id.val() != HIR_ID_NONE);
        return vec.at(id.val() - OFFSET);
    }
    [[nodiscard]] const V& cat(I id) const {
        assert(id.val() != HIR_ID_NONE);
        return vec.at(id.val() - OFFSET);
    }
    template <typename... Args> I emplace_and_get_id(Args&&... args) {
        vec.emplace_back(std::forward<Args>(args)...);
        return I{vec.size() - 1 + OFFSET}; // so just the size, but this is crucial
    }
};

/// Models a vector of an hir::Node
template <Node T> class NodeVector : public IdVecMap<typename T::id_type, T> {
  public:
    explicit NodeVector(HirSize capacity) : IdVecMap<typename T::id_type, T>(capacity) {}
};

/// Models a vector of an hir::IdIdx pointing to hir::Id
template <hir::Id I> class IdVector : public IdVecMap<typename hir::IdIdx<I>, I> {
  public:
    explicit IdVector(HirSize capacity) : IdVecMap<typename hir::IdIdx<I>, I>(capacity) {}
};

} // namespace hir

#endif
