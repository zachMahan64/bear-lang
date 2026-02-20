//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_INDEXING_HPP
#define COMPILER_HIR_INDEXING_HPP

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
concept IsId = requires { typename T::id_tag; };

template <typename T> class Id {
    HirId value;

  public:
    using id_tag = T;
    constexpr explicit Id(HirId v) : value(v) {}
    constexpr Id() : value(HIR_ID_NONE) {}
    [[nodiscard]] constexpr HirId val() const noexcept { return value; }
    friend constexpr bool operator==(Id<T> a, Id<T> b) { return a.value == b.value; }
    constexpr Id operator++() { return Id{++value}; }
    constexpr Id operator--() { return Id{--value}; }
    constexpr Id operator++(int) { return Id{value++}; }
    constexpr Id operator--(int) { return Id{value--}; }
};

struct Symbol;
struct Identifier;
struct File;
struct FileAst;
struct Scope;
struct ScopeAnon;
struct Exec;
struct Def;
struct Type;
struct GenericParam;
struct GenericArg;
struct Diagnostic;

/// primary means of tracking interned strings in the hir
using SymbolId = Id<Symbol>;

/// primary means of tracking scoped identifers like foo..bar..thing in the hir
using IdentifierId = Id<Identifier>;

/// for addressing interned file names
using FileId = Id<File>;

/// for addressing asts
using FileAstId = Id<FileAst>;

/// for addressing named scopes
using ScopeId = Id<Scope>;

/// for addressing anonymous scopes
using ScopeAnonId = Id<ScopeAnon>;

/// for addressing exec nodes
using ExecId = Id<Exec>;

/// for addressing definition nodes
using DefId = Id<Def>;

/// for addressing type nodes
using TypeId = Id<Type>;

/// for addressing generic parameter nodes
using GenericParamId = Id<GenericParam>;

/// for addressing generic argument nodes
using GenericArgId = Id<GenericArg>;

using DiagnosticId = Id<Diagnostic>;

/// to be stored in a HirSymbolId -> HirSymbol table
class Symbol {
    /// view into interned string arena for reversing symbols back into strings
    std::string_view string_view;

  public:
    using id_type = SymbolId;
    constexpr explicit Symbol(std::string_view string_view) : string_view(string_view) {}
    [[nodiscard]] constexpr std::string_view sv() const noexcept { return this->string_view; }
};

/// repsents an index into the storage for a certain Id, to allow for slicing
template <hir::IsId T> class IdIdx {
    using points_to_type = T;
    HirId value;

  public:
    using id_tag = Id<T>;
    IdIdx() : value(HIR_ID_NONE) {}
    explicit IdIdx(HirId value) : value(value) {};
    constexpr HirId val() const noexcept { return value; }
    constexpr HirId operator++() noexcept { return ++value; }
    friend constexpr bool operator==(IdIdx<T> a, IdIdx<T> b) { return a.value == b.value; }
};

/// holds an optional HirId or HirIdIdx type
template <hir::IsId T> class OptId {
    T underlying{};

  public:
    using id_tag = T;
    OptId() = default;
    OptId(T id_value) : underlying(id_value) {}
    HirId val() const { return underlying.val(); }
    constexpr T as_id() const noexcept { return underlying; }
    [[nodiscard]] bool has_value() const noexcept { return underlying.val() != HIR_ID_NONE; }
    void set(T id_value) noexcept { this->underlying = id_value; }
};

template <hir::IsId T> class IdSlice {
    using points_to_type = T;
    IdIdx<T> first_;
    HirSize len_;

  public:
    IdSlice() : first_(IdIdx<T>{}), len_{0} {}
    IdSlice(IdIdx<T> first, HirSize len) : first_(first), len_(len) {}
    constexpr IdIdx<T> first() const noexcept { return first_; }
    constexpr IdIdx<T> get(HirSize i) const noexcept { return IdIdx<T>{first_.val() + i}; }
    constexpr HirSize len() const noexcept { return len_; }

    constexpr IdIdx<T> begin() const noexcept { return first_; }

    constexpr IdIdx<T> end() const noexcept { return IdIdx<T>{first_.val() + len_}; }
};

} // namespace hir

#endif
