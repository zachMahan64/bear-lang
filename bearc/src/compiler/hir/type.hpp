//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_TYPE_HPP
#define COMPILER_HIR_TYPE_HPP

#include "compiler/hir/indexing.hpp"
#include "compiler/hir/span.hpp"
#include "utils/data_arena.hpp"
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <variant>
namespace hir {

class Context;

// ------ struct impls -------

enum class builtin_type : uint8_t {
    u8,
    i8,
    u16,
    i16,
    u32,
    i32,
    u64,
    i64,
    usize,
    charr,
    f32,
    f64,
    voidd,
    str,

};

struct TypeBuiltin {
    builtin_type type;
};

struct TypeStructure {
    DefId definition;
};

struct TypeGenericStructure {
    DefId definition;
    IdSlice<GenericArgId> slice;
};

struct TypeArr {
    TypeId inner;
    ExecId compt_size_expr;
    size_t canonical_size;
};

struct TypeSlice {
    TypeId inner;
};

struct TypeRef {
    TypeId inner;
};

struct TypePtr {
    TypeId inner;
};

struct TypeFnPtr {
    IdSlice<TypeId> param_types;
    TypeId return_type;
};

struct TypeVariadic {
    TypeId inner;
};

// ^^^^^^ struct impls ^^^^^^^^

/// main exec union
using TypeValue = std::variant<TypeBuiltin, TypeStructure, TypeGenericStructure, TypeArr, TypeSlice,
                               TypeRef, TypePtr, TypeFnPtr, TypeVariadic>;

/// main exec structure, corresponds to an hir_exec_id_t
struct Type {
    using id_type = TypeId;
    TypeValue value;
    Span span;
    CanonicalTypeId canonical;
    bool mut;
    void set_value(TypeValue value) { this->value = value; }
    template <typename V> bool holds() const noexcept { return std::holds_alternative<V>(value); }
    template <typename... Vs> bool holds_any_of() const noexcept {
        return (std::holds_alternative<Vs>(value) || ...);
    }
    template <typename V> V& as() noexcept { return std::get<V>(value); }
    template <typename V> const V& as() const noexcept { return std::get<V>(value); }
};

template <class F>
concept TypeComparisonFunctor = requires(F f, const Context& context, const Type& t1,
                                         const Type& t2) {
    typename F::value_type;
    typename F::type_comparison_functor_tag;
    // allow trivially copyable or strings as values
    requires requires {
        requires std::is_trivially_copyable_v<typename F::value_type>
                     || std::convertible_to<typename F::value_type, std::string>;
    };
    // constructor must take a const ref to a context
    { F{context} };
    // single invocation
    { f(t1) } -> std::convertible_to<typename F::value_type>;
    // double invocation
    { f(t1, t2) } -> std::convertible_to<typename F::value_type>;
    {
        f.transform(std::declval<typename F::value_type>(), std::declval<typename F::value_type>())
    } -> std::convertible_to<typename F::value_type>;
};

template <TypeComparisonFunctor F> class TypeComparator {
    const Context& context;
    OptId<TypeId> try_inner(const Type& type);

  public:
    TypeComparator(const Context& context) : context(context) {}
    typename F::value_type operator()(TypeId tid1, TypeId tid2);
    typename F::value_type operator()(TypeId tid);
};

class StructurallyEquivalentType {
    const Context& context;

  public:
    using value_type = bool;
    using type_comparison_functor_tag = value_type;
    StructurallyEquivalentType(const Context& context) : context(context) {}
    bool operator()(const Type& t1, const Type& t2) const;
    // single invocation -> mismatch => false
    bool operator()(const Type& t1) const { return false; }
    static bool transform(bool res1, bool res2) { return res1 && res2; }
};

class HashType {
    const Context& context;

  public:
    using value_type = HirSize;
    using type_comparison_functor_tag = value_type;
    HashType(const Context& context) : context(context) {}
    // probably not needed for the hasher
    size_t operator()(const Type& t1, const Type& t2) const {
        assert(false && "double invocation should not be called when hashing");
        return 0;
    }
    size_t operator()(const Type& t1) const;
    static size_t transform(size_t res1, size_t res2);
};

class CanonicalTypeTable {
    struct Entry {
        TypeId key_id;
        CanonicalTypeId val;
        Entry* next;
    };
    static constexpr size_t DEFAULT_CAP = 128;

    Context& context;
    DataArena& arena;

    Entry** buckets;
    size_t count;
    size_t capacity;

  public:
    CanonicalTypeTable(Context& context, DataArena& arena, HirSize capacity)
        : context(context), arena(arena), count{0} {
        this->capacity = (capacity > DEFAULT_CAP) ? capacity : DEFAULT_CAP;
        buckets = arena.alloc_as<Entry**>(capacity * sizeof(Entry*));

        // zero-init buckets
        memset(static_cast<void*>(buckets), 0, capacity * sizeof(Entry*));
    }
    // TODO fill out interface then impl
};

} // namespace hir

#endif
