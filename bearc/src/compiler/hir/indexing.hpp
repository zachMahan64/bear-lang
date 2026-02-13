//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_INDEXING_H
#define COMPILER_HIR_INDEXING_H

#include <concepts>
#include <stdint.h>
#include <string_view>

namespace hir {

/// defines the underlying size of indices of vectors storing HIR nodes
using HirSize = uint32_t;

/// represents a null HirId or its derivations, valued 0
inline constexpr HirSize HIR_ID_NONE = 0;

/// abstract typedef id for indexing into vectors of HIR nodes
using HirId = HirSize;

/// indicates that a type is indeed a flavor of HirId
template <typename T>
concept Id = requires(T t) {
    { t.val() } -> std::same_as<HirId>;
};

/// repsents an index into the storage for a certain Id, to allow for slicing
template <hir::Id T> class IdIdx {
    using points_to_type = T;
    HirId value;

  public:
    IdIdx() : value(HIR_ID_NONE) {}
    explicit IdIdx(HirId value) : value(value) {};
    constexpr HirId val() const noexcept { return value; }
    constexpr HirId operator++() noexcept { return ++value; }
    friend constexpr bool operator==(IdIdx<T> a, IdIdx<T> b) { return a.value == b.value; }
};

/// primary means of tracking interned strings in the hir
struct SymbolId {
    HirId value;
    constexpr explicit SymbolId(HirId v) : value(v) {}
    constexpr SymbolId() : value(HIR_ID_NONE) {}
    [[nodiscard]] constexpr HirId val() const noexcept { return value; }
    friend constexpr bool operator==(SymbolId a, SymbolId b) { return a.value == b.value; }
};

/// primary means of tracking interned strings in the hir
struct IdentifierId {
    HirId value;
    constexpr explicit IdentifierId(HirId v) : value(v) {}
    constexpr IdentifierId() : value(HIR_ID_NONE) {}
    [[nodiscard]] constexpr HirId val() const noexcept { return value; }
    friend constexpr bool operator==(IdentifierId a, IdentifierId b) { return a.value == b.value; }
};

/// to be stored in a HirSymbolId -> HirSymbol table
class Symbol {
    /// view into interned string arena for reversing symbols back into strings
    std::string_view string_view;

  public:
    using id_type = SymbolId;
    constexpr explicit Symbol(std::string_view string_view) : string_view(string_view) {}
    [[nodiscard]] constexpr std::string_view sv() const noexcept { return this->string_view; }
};

/// for addressing interned file names
class FileId {
    HirId value;

  public:
    constexpr explicit FileId(HirId v) : value(v) {}
    constexpr FileId() : value(HIR_ID_NONE) {}
    [[nodiscard]] constexpr HirId val() const noexcept { return value; }
    friend constexpr bool operator==(FileId a, FileId b) { return a.value == b.value; }
    constexpr FileId operator++() { return FileId{++value}; }
};

/// for addressing asts
class FileAstId {
    HirId value;

  public:
    constexpr explicit FileAstId(HirId v) : value(v) {}
    constexpr FileAstId() : value(HIR_ID_NONE) {}
    [[nodiscard]] constexpr HirId val() const noexcept { return value; }
    friend constexpr bool operator==(FileAstId a, FileAstId b) { return a.value == b.value; }
};

/// for addressing named scopes
class ScopeId {
    HirId value;

  public:
    constexpr explicit ScopeId(HirId v) : value(v) {}
    constexpr ScopeId() : value(HIR_ID_NONE) {}
    [[nodiscard]] constexpr HirId val() const noexcept { return value; }
    friend constexpr bool operator==(ScopeId a, ScopeId b) { return a.value == b.value; }
};

/// for addressing anonymous scopes
class ScopeAnonId {
    HirId value;

  public:
    constexpr explicit ScopeAnonId(HirId v) : value(v) {}
    constexpr ScopeAnonId() : value(HIR_ID_NONE) {}
    [[nodiscard]] constexpr HirId val() const noexcept { return value; }
    friend constexpr bool operator==(ScopeAnonId a, ScopeAnonId b) { return a.value == b.value; }
};

/// for addressing exec nodes
class ExecId {
    HirId value;

  public:
    constexpr explicit ExecId(HirId v) : value(v) {}
    constexpr ExecId() : value(HIR_ID_NONE) {}
    [[nodiscard]] constexpr HirId val() const noexcept { return value; }
    friend constexpr bool operator==(ExecId a, ExecId b) { return a.value == b.value; }
};

/// for addressing definition nodes
class DefId {
    HirId value;

  public:
    constexpr explicit DefId(HirId v) : value(v) {}
    constexpr DefId() : value(HIR_ID_NONE) {}
    [[nodiscard]] constexpr HirId val() const noexcept { return value; }
    friend constexpr bool operator==(DefId a, DefId b) { return a.value == b.value; }
};

/// for addressing type nodes
class TypeId {
    HirId value;

  public:
    constexpr explicit TypeId(HirId v) : value(v) {}
    constexpr TypeId() : value(HIR_ID_NONE) {}
    [[nodiscard]] constexpr HirId val() const noexcept { return value; }
    friend constexpr bool operator==(TypeId a, TypeId b) { return a.value == b.value; }
};

/// for addressing generic parameter nodes
class GenericParamId {
    HirId value;

  public:
    constexpr explicit GenericParamId(HirId v) : value(v) {}
    constexpr GenericParamId() : value(HIR_ID_NONE) {}
    [[nodiscard]] constexpr HirId val() const noexcept { return value; }
    friend constexpr bool operator==(GenericParamId a, GenericParamId b) {
        return a.value == b.value;
    }
};

/// for addressing generic argument nodes
class GenericArgId {
    HirId value;

  public:
    constexpr explicit GenericArgId(HirId v) : value(v) {}
    constexpr GenericArgId() : value(HIR_ID_NONE) {}
    [[nodiscard]] constexpr HirId val() const noexcept { return value; }
    friend constexpr bool operator==(GenericArgId a, GenericArgId b) { return a.value == b.value; }
};

/// for addressing parameter nodes
class ParamId {
    HirId value;

  public:
    constexpr explicit ParamId(HirId v) : value(v) {}
    constexpr ParamId() : value(HIR_ID_NONE) {}
    [[nodiscard]] constexpr HirId val() const noexcept { return value; }
    friend constexpr bool operator==(ParamId a, ParamId b) { return a.value == b.value; }
};
/// holds an optional HirId or HirIdIdx type
template <hir::Id T> class OptId {
    T underlying{};

  public:
    OptId() = default;
    OptId(T id_value) : underlying(id_value) {}
    HirId val() const { return underlying.val(); }
    constexpr T as_id() const noexcept { return underlying; }
    [[nodiscard]] bool has_value() const noexcept { return underlying.val() != HIR_ID_NONE; }
    void set(T id_value) noexcept { this->underlying = id_value; }
};

template <hir::Id T> class IdSlice {
    using points_to_type = T;
    IdIdx<T> first_;
    HirSize len_;

  public:
    IdSlice() : first_(IdIdx<T>{}), len_{0} {}
    IdSlice(IdIdx<T> first, HirSize len) : first_(first), len_(len) {}
    constexpr IdIdx<T> first() const noexcept { return first_; }
    constexpr IdIdx<T> get(HirSize i) const noexcept { return IdIdx<T>{first_.val() + i}; }
    constexpr HirSize len() const noexcept { return len_; }

    constexpr IdIdx<T> begin() const noexcept { return first; }

    constexpr IdIdx<T> end() const noexcept { return IdIdx<T>{first_.val() + len_}; }
};

} // namespace hir

#endif
