//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/exec.hpp"
#include "compiler/hir/context.hpp"
#include "compiler/hir/diagnostic.hpp"
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdint>
namespace hir {

std::optional<ExecExprComptConstant>
ExecExprComptConstant::try_implicit_convert_to(builtin_type type) {
    auto maybe_up = try_up_convert_to(type);
    return maybe_up.has_value() ? maybe_up : try_down_convert_to(type);
}

std::optional<ExecExprComptConstant> ExecExprComptConstant::try_up_convert_to(builtin_type type) {
    using OptConst = std::optional<ExecExprComptConstant>;
    auto to_optconst = +[](ConstantValue constval) -> OptConst {
        return OptConst{ExecExprComptConstant{constval}};
    };
    auto none = +[] { return OptConst{}; };
    switch (value.index()) {
    case 0:
        // builtin_type::str;
        switch (type) {
        case builtin_type::str:
            return *this;
        case builtin_type::u8:
        case builtin_type::i8:
        case builtin_type::u16:
        case builtin_type::i16:
        case builtin_type::u32:
        case builtin_type::i32:
        case builtin_type::u64:
        case builtin_type::i64:
        case builtin_type::usize:
        case builtin_type::charr:
        case builtin_type::f32:
        case builtin_type::f64:
        case builtin_type::voidd:
        case builtin_type::nullpointer:
        case builtin_type::boolean:
            break;
        }
        break;
    case 1: {
        // builtin_type::i8
        const int8_t val = as<int8_t>();
        switch (type) {
        case builtin_type::u8:
            return to_optconst(ConstantValue{static_cast<uint8_t>(val)});
        case builtin_type::i8:
            return *this;
        case builtin_type::u16:
            return to_optconst(ConstantValue{static_cast<uint16_t>(val)});
        case builtin_type::i16:
            return to_optconst(ConstantValue{static_cast<int16_t>(val)});
        case builtin_type::u32:
            return to_optconst(ConstantValue{static_cast<uint32_t>(val)});
        case builtin_type::i32:
            return to_optconst(ConstantValue{static_cast<int32_t>(val)});
        case builtin_type::u64:
            return to_optconst(ConstantValue{static_cast<uint64_t>(val)});
        case builtin_type::i64:
            return to_optconst(ConstantValue{static_cast<int64_t>(val)});
        case builtin_type::usize:
            return to_optconst(ConstantValue{static_cast<size_t>(val)});
        case builtin_type::charr:
            return to_optconst(ConstantValue{static_cast<char>(val)});
        case builtin_type::f32:
            return to_optconst(ConstantValue{static_cast<float>(val)});
        case builtin_type::f64:
            return to_optconst(ConstantValue{static_cast<double>(val)});
        case builtin_type::voidd:
        case builtin_type::str:
        case builtin_type::nullpointer:
            return none();
        case builtin_type::boolean:
            return to_optconst(ConstantValue{static_cast<bool>(val)});
        }
    }
    case 2: {
        // builtin_type::u8;
        const uint8_t val = as<uint8_t>();
        switch (type) {
        case builtin_type::u8:
            return *this;
        case builtin_type::i8:
            return to_optconst(ConstantValue{static_cast<uint8_t>(val)});
        case builtin_type::u16:
            return to_optconst(ConstantValue{static_cast<uint16_t>(val)});
        case builtin_type::i16:
            return to_optconst(ConstantValue{static_cast<int16_t>(val)});
        case builtin_type::u32:
            return to_optconst(ConstantValue{static_cast<uint32_t>(val)});
        case builtin_type::i32:
            return to_optconst(ConstantValue{static_cast<int32_t>(val)});
        case builtin_type::u64:
            return to_optconst(ConstantValue{static_cast<uint64_t>(val)});
        case builtin_type::i64:
            return to_optconst(ConstantValue{static_cast<int64_t>(val)});
        case builtin_type::usize:
            return to_optconst(ConstantValue{static_cast<size_t>(val)});
        case builtin_type::charr:
            return to_optconst(ConstantValue{static_cast<char>(val)});
        case builtin_type::f32:
            return to_optconst(ConstantValue{static_cast<float>(val)});
        case builtin_type::f64:
            return to_optconst(ConstantValue{static_cast<double>(val)});
        case builtin_type::voidd:
        case builtin_type::str:
        case builtin_type::nullpointer:
            return none();
        case builtin_type::boolean:
            return to_optconst(ConstantValue{static_cast<bool>(val)});
        }
    }
    case 3: {
        // builtin_type::i16;
        const int16_t val = as<int16_t>();
        switch (type) {
        case builtin_type::u8:
        case builtin_type::i8:
            return none();
        case builtin_type::u16:
            return to_optconst(ConstantValue{static_cast<uint16_t>(val)});
        case builtin_type::i16:
            return *this;
        case builtin_type::u32:
            return to_optconst(ConstantValue{static_cast<uint32_t>(val)});
        case builtin_type::i32:
            return to_optconst(ConstantValue{static_cast<int32_t>(val)});
        case builtin_type::u64:
            return to_optconst(ConstantValue{static_cast<uint64_t>(val)});
        case builtin_type::i64:
            return to_optconst(ConstantValue{static_cast<int64_t>(val)});
        case builtin_type::usize:
            return to_optconst(ConstantValue{static_cast<size_t>(val)});
        case builtin_type::charr:
            return to_optconst(ConstantValue{static_cast<char>(val)});
        case builtin_type::f32:
            return to_optconst(ConstantValue{static_cast<float>(val)});
        case builtin_type::f64:
            return to_optconst(ConstantValue{static_cast<double>(val)});
        case builtin_type::voidd:
        case builtin_type::str:
        case builtin_type::nullpointer:
            return none();
        case builtin_type::boolean:
            return to_optconst(ConstantValue{static_cast<bool>(val)});
        }
    }
    case 4: {
        // builtin_type::u16;
        const uint16_t val = as<uint16_t>();
        switch (type) {
        case builtin_type::u8:
        case builtin_type::i8:
            return none();
        case builtin_type::u16:
            return *this;
        case builtin_type::i16:
            return to_optconst(ConstantValue{static_cast<int16_t>(val)});
        case builtin_type::u32:
            return to_optconst(ConstantValue{static_cast<uint32_t>(val)});
        case builtin_type::i32:
            return to_optconst(ConstantValue{static_cast<int32_t>(val)});
        case builtin_type::u64:
            return to_optconst(ConstantValue{static_cast<uint64_t>(val)});
        case builtin_type::i64:
            return to_optconst(ConstantValue{static_cast<int64_t>(val)});
        case builtin_type::usize:
            return to_optconst(ConstantValue{static_cast<size_t>(val)});
        case builtin_type::charr:
            return to_optconst(ConstantValue{static_cast<char>(val)});
        case builtin_type::f32:
            return to_optconst(ConstantValue{static_cast<float>(val)});
        case builtin_type::f64:
            return to_optconst(ConstantValue{static_cast<double>(val)});
        case builtin_type::voidd:
        case builtin_type::str:
        case builtin_type::nullpointer:
            return none();
        case builtin_type::boolean:
            return to_optconst(ConstantValue{static_cast<bool>(val)});
        }
    }
    case 5: {
        // builtin_type::i32;
        const int32_t val = as<int32_t>();
        switch (type) {
        case builtin_type::u8:
        case builtin_type::i8:
        case builtin_type::u16:
        case builtin_type::i16:
            return none();
        case builtin_type::u32:
            return to_optconst(ConstantValue{static_cast<uint32_t>(val)});
        case builtin_type::i32:
            return *this;
        case builtin_type::u64:
            return to_optconst(ConstantValue{static_cast<uint64_t>(val)});
        case builtin_type::i64:
            return to_optconst(ConstantValue{static_cast<int64_t>(val)});
        case builtin_type::usize:
            return to_optconst(ConstantValue{static_cast<size_t>(val)});
        case builtin_type::charr:
            return to_optconst(ConstantValue{static_cast<char>(val)});
        case builtin_type::f32:
            return to_optconst(ConstantValue{static_cast<float>(val)});
        case builtin_type::f64:
            return to_optconst(ConstantValue{static_cast<double>(val)});
        case builtin_type::voidd:
        case builtin_type::str:
        case builtin_type::nullpointer:
            return none();
        case builtin_type::boolean:
            return to_optconst(ConstantValue{static_cast<bool>(val)});
        }
    }
    case 6: {
        // builtin_type::u32;
        const uint32_t val = as<uint32_t>();
        switch (type) {
        case builtin_type::u8:
        case builtin_type::i8:
        case builtin_type::u16:
        case builtin_type::i16:
            return none();
        case builtin_type::u32:
            return *this;
        case builtin_type::i32:
            return to_optconst(ConstantValue{static_cast<int32_t>(val)});
        case builtin_type::u64:
            return to_optconst(ConstantValue{static_cast<uint64_t>(val)});
        case builtin_type::i64:
            return to_optconst(ConstantValue{static_cast<int64_t>(val)});
        case builtin_type::usize:
            return to_optconst(ConstantValue{static_cast<size_t>(val)});
        case builtin_type::charr:
            return to_optconst(ConstantValue{static_cast<char>(val)});
        case builtin_type::f32:
            return to_optconst(ConstantValue{static_cast<float>(val)});
        case builtin_type::f64:
            return to_optconst(ConstantValue{static_cast<double>(val)});
        case builtin_type::voidd:
        case builtin_type::str:
        case builtin_type::nullpointer:
            return none();
        case builtin_type::boolean:
            return to_optconst(ConstantValue{static_cast<bool>(val)});
        }
    }
    case 7: {
        // builtin_type::i64;
        const int64_t val = as<int64_t>();
        switch (type) {
        case builtin_type::u8:
        case builtin_type::i8:
        case builtin_type::u16:
        case builtin_type::i16:
        case builtin_type::u32:
        case builtin_type::i32:
            return none();
        case builtin_type::u64:
            return to_optconst(ConstantValue{static_cast<uint64_t>(val)});
        case builtin_type::i64:
            return *this;
        case builtin_type::usize:
            return to_optconst(ConstantValue{static_cast<size_t>(val)});
        case builtin_type::charr:
            return to_optconst(ConstantValue{static_cast<char>(val)});
        case builtin_type::f32:
            return to_optconst(ConstantValue{static_cast<float>(val)});
        case builtin_type::f64:
            return to_optconst(ConstantValue{static_cast<double>(val)});
        case builtin_type::voidd:
        case builtin_type::str:
        case builtin_type::nullpointer:
            return none();
        case builtin_type::boolean:
            return to_optconst(ConstantValue{static_cast<bool>(val)});
        }
    }
    case 8: {
        // builtin_type::u64;
        const uint64_t val = as<uint64_t>();
        switch (type) {
        case builtin_type::u8:
        case builtin_type::i8:
        case builtin_type::u16:
        case builtin_type::i16:
        case builtin_type::u32:
        case builtin_type::i32:
            return none();
        case builtin_type::u64:
            return *this;
        case builtin_type::i64:
            return to_optconst(ConstantValue{static_cast<uint64_t>(val)});
        case builtin_type::usize:
            return to_optconst(ConstantValue{static_cast<size_t>(val)});
        case builtin_type::charr:
            return to_optconst(ConstantValue{static_cast<char>(val)});
        case builtin_type::f32:
            return to_optconst(ConstantValue{static_cast<float>(val)});
        case builtin_type::f64:
            return to_optconst(ConstantValue{static_cast<double>(val)});
        case builtin_type::voidd:
        case builtin_type::str:
        case builtin_type::nullpointer:
            return none();
        case builtin_type::boolean:
            return to_optconst(ConstantValue{static_cast<bool>(val)});
        }
    }
    case 9: {
        // builtin_type::charr;
        const char val = as<char>();
        switch (type) {
        case builtin_type::u8:
            return to_optconst(ConstantValue{static_cast<uint8_t>(val)});
        case builtin_type::i8:
            return to_optconst(ConstantValue{static_cast<int8_t>(val)});
        case builtin_type::u16:
            return to_optconst(ConstantValue{static_cast<uint16_t>(val)});
        case builtin_type::i16:
            return to_optconst(ConstantValue{static_cast<int16_t>(val)});
        case builtin_type::u32:
            return to_optconst(ConstantValue{static_cast<uint32_t>(val)});
        case builtin_type::i32:
            return to_optconst(ConstantValue{static_cast<int32_t>(val)});
        case builtin_type::u64:
            return to_optconst(ConstantValue{static_cast<uint64_t>(val)});
        case builtin_type::i64:
            return to_optconst(ConstantValue{static_cast<int64_t>(val)});
        case builtin_type::usize:
            return to_optconst(ConstantValue{static_cast<size_t>(val)});
        case builtin_type::charr:
            return to_optconst(ConstantValue{static_cast<char>(val)});
        case builtin_type::f32:
            return to_optconst(ConstantValue{static_cast<float>(val)});
        case builtin_type::f64:
            return to_optconst(ConstantValue{static_cast<double>(val)});
        case builtin_type::voidd:
        case builtin_type::str:
        case builtin_type::nullpointer:
            return none();
        case builtin_type::boolean:
            return to_optconst(ConstantValue{static_cast<bool>(val)});
        }
    }
    case 10: {
        // builtin_type::f32;
        const float val = as<float>();
        switch (type) {
        case builtin_type::u8:
        case builtin_type::i8:
        case builtin_type::u16:
        case builtin_type::i16:
        case builtin_type::u32:
        case builtin_type::i32:
        case builtin_type::u64:
        case builtin_type::i64:
        case builtin_type::usize:
        case builtin_type::charr:
            return none();
        case builtin_type::f32:
            return *this;
        case builtin_type::f64:
            return to_optconst(ConstantValue{static_cast<double>(val)});
            break;
        case builtin_type::voidd:
        case builtin_type::str:
        case builtin_type::nullpointer:
            return none();
            break;
        case builtin_type::boolean:
            return to_optconst(ConstantValue{static_cast<bool>(val)});
            break;
        }
    }
    case 11: {
        // builtin_type::f64;
        const double val = as<double>();
        switch (type) {
        case builtin_type::u8:
        case builtin_type::i8:
        case builtin_type::u16:
        case builtin_type::i16:
        case builtin_type::u32:
        case builtin_type::i32:
        case builtin_type::u64:
        case builtin_type::i64:
        case builtin_type::usize:
        case builtin_type::charr:
        case builtin_type::f32:
            return none();
        case builtin_type::f64:
            return *this;
            break;
        case builtin_type::voidd:
        case builtin_type::str:
        case builtin_type::nullpointer:
            return none();
            break;
        case builtin_type::boolean:
            return to_optconst(ConstantValue{static_cast<bool>(val)});
            break;
        }
    }
    case 12: {
        // builtin_type::nullpointer;
        switch (type) {
        case builtin_type::u8:
        case builtin_type::i8:
        case builtin_type::u16:
        case builtin_type::i16:
        case builtin_type::u32:
        case builtin_type::i32:
        case builtin_type::u64:
        case builtin_type::i64:
        case builtin_type::usize:
        case builtin_type::charr:
        case builtin_type::f32:
        case builtin_type::f64:
        case builtin_type::voidd:
        case builtin_type::str:
            return none();
        case builtin_type::nullpointer:
            return *this;
            break;
        case builtin_type::boolean:
            return to_optconst(ConstantValue{false});
            break;
        }
    }
    case 13: {
        // builtin_type::boolean;
        switch (type) {
        case builtin_type::u8:
        case builtin_type::i8:
        case builtin_type::u16:
        case builtin_type::i16:
        case builtin_type::u32:
        case builtin_type::i32:
        case builtin_type::u64:
        case builtin_type::i64:
        case builtin_type::usize:
        case builtin_type::charr:
        case builtin_type::f32:
        case builtin_type::f64:
        case builtin_type::voidd:
        case builtin_type::str:
        case builtin_type::nullpointer:
            return none();
        case builtin_type::boolean:
            return *this;
            break;
        }
    }
    default:
        break;
    }
    assert(false && "unconsidered builtin type");
    return OptConst{};
}
std::optional<ExecExprComptConstant> ExecExprComptConstant::try_down_convert_to(builtin_type type) {

    using OptConst = std::optional<ExecExprComptConstant>;

    auto to_optconst
        = +[](ConstantValue v) -> OptConst { return OptConst{ExecExprComptConstant{v}}; };

    auto none = +[] { return OptConst{}; };

    auto fits_signed = [](int64_t v, int64_t lo, int64_t hi) { return v >= lo && v <= hi; };

    auto fits_unsigned = [](uint64_t v, uint64_t hi) { return v <= hi; };

    auto fits_float_int = [](double v) { return std::floor(v) == v; };

    switch (value.index()) {

    // i64
    case 7: {
        int64_t v = as<int64_t>();

        switch (type) {

        case builtin_type::i8:
            if (fits_signed(v, INT8_MIN, INT8_MAX)) {
                return to_optconst(ConstantValue{static_cast<int8_t>(v)});
            }
            return none();

        case builtin_type::u8:
            if (fits_signed(v, 0, UINT8_MAX)) {
                return to_optconst(ConstantValue{static_cast<uint8_t>(v)});
            }
            return none();

        case builtin_type::i16:
            if (fits_signed(v, INT16_MIN, INT16_MAX)) {
                return to_optconst(ConstantValue{static_cast<int16_t>(v)});
            }
            return none();

        case builtin_type::u16:
            if (fits_signed(v, 0, UINT16_MAX)) {
                return to_optconst(ConstantValue{static_cast<uint16_t>(v)});
            }
            return none();

        case builtin_type::i32:
            if (fits_signed(v, INT32_MIN, INT32_MAX)) {
                return to_optconst(ConstantValue{static_cast<int32_t>(v)});
            }
            return none();

        case builtin_type::u32:
            if (fits_signed(v, 0, UINT32_MAX)) {
                return to_optconst(ConstantValue{static_cast<uint32_t>(v)});
            }
            return none();

        case builtin_type::i64:
            return *this;

        case builtin_type::u64:
            if (v >= 0) {
                return to_optconst(ConstantValue{static_cast<uint64_t>(v)});
            }
            return none();

        case builtin_type::f32:
            return to_optconst(ConstantValue{static_cast<float>(v)});

        case builtin_type::f64:
            return to_optconst(ConstantValue{static_cast<double>(v)});

        case builtin_type::boolean:
            return to_optconst(ConstantValue{static_cast<bool>(v)});

        default:
            return none();
        }
    }

    // u64
    case 8: {
        uint64_t v = as<uint64_t>();

        switch (type) {

        case builtin_type::u8:
            if (fits_unsigned(v, UINT8_MAX)) {
                return to_optconst(ConstantValue{static_cast<uint8_t>(v)});
            }
            return none();

        case builtin_type::i8:
            if (v <= INT8_MAX) {
                return to_optconst(ConstantValue{static_cast<int8_t>(v)});
            }
            return none();

        case builtin_type::u16:
            if (fits_unsigned(v, UINT16_MAX)) {
                return to_optconst(ConstantValue{static_cast<uint16_t>(v)});
            }
            return none();

        case builtin_type::i16:
            if (v <= INT16_MAX) {
                return to_optconst(ConstantValue{static_cast<int16_t>(v)});
            }
            return none();

        case builtin_type::u32:
            if (fits_unsigned(v, UINT32_MAX)) {
                return to_optconst(ConstantValue{static_cast<uint32_t>(v)});
            }
            return none();

        case builtin_type::i32:
            if (v <= INT32_MAX) {
                return to_optconst(ConstantValue{static_cast<int32_t>(v)});
            }
            return none();

        case builtin_type::u64:
            return *this;

        case builtin_type::i64:
            if (v <= INT64_MAX) {
                return to_optconst(ConstantValue{static_cast<int64_t>(v)});
            }
            return none();

        case builtin_type::f32:
            return to_optconst(ConstantValue{static_cast<float>(v)});

        case builtin_type::f64:
            return to_optconst(ConstantValue{static_cast<double>(v)});

        case builtin_type::boolean:
            return to_optconst(ConstantValue{static_cast<bool>(v)});

        default:
            return none();
        }
    }

    // f64
    case 11: {
        double v = as<double>();

        switch (type) {

        case builtin_type::i32:
            if (fits_float_int(v) && v >= INT32_MIN && v <= INT32_MAX) {
                return to_optconst(ConstantValue{static_cast<int32_t>(v)});
            }
            return none();

        case builtin_type::i64:
            if (fits_float_int(v) && v >= INT64_MIN && v <= INT64_MAX) {
                return to_optconst(ConstantValue{static_cast<int64_t>(v)});
            }
            return none();

        case builtin_type::f32:
            if (v >= FLT_MIN && v <= FLT_MAX) {
                return to_optconst(ConstantValue{static_cast<float>(v)});
            }
            return none();

        case builtin_type::f64:
            return *this;

        case builtin_type::boolean:
            return to_optconst(ConstantValue{static_cast<bool>(v)});

        default:
            return none();
        }
    }

    default:
        break;
    }

    return none();
}

Exec::Exec(Context& ctx, ExecValue value, Span span, bool should_be_compt)
    : value{value}, span{span} {
    bool truely_compt = can_be_compt(ctx);
    if (should_be_compt && !truely_compt) {
        ctx.emplace_diagnostic(span, diag_code::value_cannot_be_compt, diag_type::error);
    }
    this->compt = truely_compt && should_be_compt;
}

bool Exec::is_equivalent(const Context& ctx, ExecId eid1, ExecId eid2) {
    // same ExecId always means equivalent
    if (eid1 == eid2) {
        return true;
    }
    const Exec& e1 = ctx.exec(eid1);
    const Exec& e2 = ctx.exec(eid2);
    const bool both_compt = e1.compt && e2.compt;
    if (!both_compt) {
        return false;
    }
    if (e1.holds_same<ExecExprComptConstant>(e2)) {
        const ExecExprComptConstant& lit1 = e1.as<ExecExprComptConstant>();
        const ExecExprComptConstant& lit2 = e2.as<ExecExprComptConstant>();
        return lit1.variant_value_equals(lit2);
    }
    return false;
}

bool Exec::can_be_compt(const Context& ctx) {
    auto get_d = [&](DefId did) { return ctx.def(did); };
    auto get_e = [&](ExecId eid) { return ctx.exec(eid); };
    auto eidx_to_e = [&](IdIdx<ExecId> eid) { return ctx.exec(eid); };
    auto vs = Ovld{
        [&](const ExecBlock& t) -> bool { return false; },
        [&](const ExecExprStmt& t) -> bool { return false; },
        [&](const ExecBreakStmt& t) -> bool { return false; },
        [&](const ExecIfStmt& t) -> bool { return false; },
        [&](const ExecLoopStmt& t) -> bool { return false; },
        [&](const ExecReturnStmt& t) -> bool { return false; },
        [&](const ExecYieldStmt& t) -> bool { return false; },
        // exprs
        [&](const ExecExprIdentifier& t) -> bool { return get_d(t.identifier).compt; },
        [&](const ExecExprComptConstant& t) -> bool { return true; },
        [&](const ExecExprListLiteral& t) -> bool {
            // just check each elem
            for (auto eidx = t.elems.begin(); eidx != t.elems.end(); eidx++) {
                if (!eidx_to_e(eidx).compt) {
                    return false;
                }
            }
            return true;
        },
        [&](const ExecExprAssignMove& t) -> bool {
            return get_e(t.lhs).compt && get_e(t.rhs).compt;
        },
        [&](const ExecExprAssignEqual& t) -> bool {
            return get_e(t.lhs).compt && get_e(t.rhs).compt;
        },
        [&](const ExecExprIs& t) -> bool {
            return get_e(t.variant_instance).compt && get_e(t.variant_decomp).compt;
        },
        [&](const ExecExprMemberAccess& t) -> bool {
            return get_e(t.owner).compt && get_e(t.member).compt;
        },
        [&](const ExecExprPointerMemberAccess& t) -> bool {
            return get_e(t.owner).compt && get_e(t.member).compt;
        },
        [&](const ExecExprBinary& t) -> bool { return get_e(t.lhs).compt && get_e(t.rhs).compt; },
        [&](const ExecExprCast& t) -> bool { return get_e(t.expr).compt; },
        [&](const ExecExprPreUnary& t) -> bool { return get_e(t.expr).compt; },
        [&](const ExecExprPostUnary& t) -> bool { return get_e(t.expr).compt; },
        [&](const ExecExprSubscript& t) -> bool {
            return get_e(t.base).compt && get_e(t.index).compt;
        },
        [&](const ExecExprFnCall& t) -> bool {
            const bool callee = get_e(t.callee).compt;
            // check all args, ret false if one isn't compt
            for (auto eidx = t.args.begin(); eidx != t.args.end(); eidx++) {
                if (!eidx_to_e(eidx).compt) {
                    return false;
                }
            }
            return callee;
        },
        [&](const ExecExprBorrow& t) -> bool { return get_e(t.borrowee).compt; },
        [&](const ExecExprDeref& t) -> bool { return get_e(t.expr).compt; },
        [&](const ExecExprStructInit& t) -> bool {
            for (auto eidx = t.member_inits.begin(); eidx != t.member_inits.end(); eidx++) {
                if (!eidx_to_e(eidx).compt) {
                    return false;
                }
            }
            return true;
        },
        [&](const ExecExprStructMemberInit& t) -> bool { return get_e(t.value).compt; },
        // full compt ctrl no for now
        [&](const ExecExprClosure& t) -> bool { return false; },
        [&](const ExecExprVariantDecomp& t) -> bool { return false; },
        [&](const ExecExprMatch& t) -> bool { return false; },
        [&](const ExecExprMatchBranch& t) -> bool { return false; },
    };
    return visit(vs);
}

} // namespace hir
