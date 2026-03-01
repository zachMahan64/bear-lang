//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/type.hpp"
#include "compiler/hir/context.hpp"
#include <variant>

namespace hir {

bool SameType::operator()(const Type& t1, const Type& t2) const {
    auto vs = Ovld{
        [&](const TypeBuiltin& t) -> bool {
            if (!t2.holds<TypeBuiltin>()) {
                return false;
            }
            return t.type == t2.as<TypeBuiltin>().type;
        },
        [&](const TypeStructure& t) -> bool {
            if (!t2.holds<TypeStructure>()) {
                return false;
            }
            return t.definition == t2.as<TypeStructure>().definition;
        },
        [&](const TypeGenericStructure& t) -> bool {
            if (!t2.holds<TypeGenericStructure>()) {
                return false;
            }
            return t.definition == t2.as<TypeGenericStructure>().definition;
        },
        [&](const TypeArr& t) -> bool {
            if (!t2.holds<TypeArr>()) {
                return false;
            }
            return t.canonical_size == t2.as<TypeArr>().canonical_size;
        },
        [&](const TypeSlice& t) -> bool { return t2.holds<TypeSlice>(); },
        [&](const TypeRef& t) -> bool { return t2.holds<TypeRef>(); },
        [&](const TypePtr& t) -> bool { return t2.holds<TypePtr>(); },
        [&](const TypeFnPtr& t) -> bool {
            if (!t2.holds<TypeFnPtr>()) {
                return false;
            }
            bool rt_match = t.return_type == t2.as<TypeFnPtr>().return_type;
            bool arity_match = t.param_types.len() == t2.as<TypeFnPtr>().param_types.len();
            if (!rt_match || !arity_match) {
                return false;
            }
            auto t_start = t.param_types.begin();
            auto t2_start = t2.as<TypeFnPtr>().param_types.begin();
            auto len
                = t.param_types.len(); // guranteed to match since we already passed arity check
            // iterate thru params and return false on mismatch
            for (HirSize i = 0; i < len; i++) {
                auto tid = context.type_id(t_start.at(i));
                auto tid2 = context.type_id(t2_start.at(i));
                if (!TypeComparator<SameType>{context}(tid, tid2)) {
                    return false;
                }
            }
            // all matched, so return true
            return true;
        },
        [&](const TypeVariadic& t) -> bool { return t2.holds<TypeVariadic>(); },

    };
    if (t1.mut != t2.mut) {
        return false;
    }
    return std::visit(vs, t1.value);
}
template <TypeComparisonFunctor F> OptId<TypeId> TypeComparator<F>::try_inner(const Type& type) {
    using OTid = OptId<TypeId>;
    TypeValue type_value = type.value;
    auto vs = Ovld{
        [&](const TypeBuiltin& t) -> OTid { return OTid{}; },
        [&](const TypeStructure& t) -> OTid { return OTid{}; },
        [&](const TypeGenericStructure& t) -> OTid { return OTid{}; },
        [&](const TypeArr& t) -> OTid { return t.inner; },
        [&](const TypeSlice& t) -> OTid { return t.inner; },
        [&](const TypeRef& t) -> OTid { return t.inner; },
        [&](const TypePtr& t) -> OTid { return t.inner; },
        [&](const TypeFnPtr& t) -> OTid { return OTid{}; },
        [&](const TypeVariadic& t) -> OTid { return t.inner; },

    };
    return std::visit(vs, type_value);
}

template <TypeComparisonFunctor F>
typename F::value_type TypeComparator<F>::operator()(TypeId tid1, TypeId tid2) {
    auto get_t = [&](TypeId tid) { return context.ctype(tid); };

    auto t1 = get_t(tid1);
    auto t2 = get_t(tid2);

    const auto comparison_functor = F{context};

    // get first value up-front (prevents collection bugs from a default initialized collector)
    typename F::value_type collector = comparison_functor(t1, t2);

    OptId<TypeId> maybe_tid1{tid1};
    OptId<TypeId> maybe_tid2{tid2};

    while (maybe_tid1.has_value() && maybe_tid2.has_value()) {
        collector = F::transform(collector, comparison_functor(t1, t2));
        maybe_tid1 = try_inner(t1);
        maybe_tid2 = try_inner(t2);
        if (maybe_tid1.has_value()) {
            t1 = get_t(maybe_tid1.as_id());
        }
        if (maybe_tid2.has_value()) {
            t2 = get_t(maybe_tid2.as_id());
        }
    }
    // return conditional single-invocation
    return maybe_tid1.has_value() ? F::transform(collector, comparison_functor(t1))
                                  : F::transform(collector, comparison_functor(t2));
}
template class TypeComparator<SameType>;

} // namespace hir
