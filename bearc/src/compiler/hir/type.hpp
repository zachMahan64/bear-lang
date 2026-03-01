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
#include <concepts>
#include <cstdint>
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
    std::is_trivially_copyable_v<typename F::value_type>;

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
};
class SameType {
    const Context& context;

  public:
    using value_type = bool;
    using type_comparison_functor_tag = value_type;
    SameType(const Context& context) : context(context) {}
    bool operator()(const Type& t1, const Type& t2) const;
    // single invocation -> mismatch => false
    bool operator()(const Type& t1) const { return false; }
    static bool transform(bool res1, bool res2) { return res1 && res2; }
};

} // namespace hir

#endif
