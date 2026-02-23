// NOLINTBEGIN(readability-magic-numbers,readability-identifier-length) — test file
#include "strong-types/safe_math.hpp"
#include "strong-types/si_scaled_literals.hpp"

#include <climits>
#include <cstdint>

using namespace strong_types;
using namespace strong_types::si_scaled_literals;

// ---- safe_multiply: normal cases ----

static_assert(safe_multiply(6, 7).value() == 42, "6 * 7 = 42");
static_assert(safe_multiply(0, 100).value() == 0, "0 * 100 = 0");
static_assert(safe_multiply(100, 0).value() == 0, "100 * 0 = 0");
static_assert(safe_multiply(-3, 4).value() == -12, "signed: -3 * 4 = -12");
static_assert(safe_multiply(-3, -4).value() == 12, "signed: -3 * -4 = 12");

// ---- safe_multiply: overflow ----

static_assert(safe_multiply(INT_MAX, 2).error() == ArithmeticErrc::overflow, "INT_MAX * 2 overflows");
static_assert(safe_multiply(2, INT_MAX).error() == ArithmeticErrc::overflow, "2 * INT_MAX overflows");
static_assert(safe_multiply(INT_MIN, -1).error() == ArithmeticErrc::overflow, "INT_MIN * -1 overflows");
static_assert(safe_multiply(-1, INT_MIN).error() == ArithmeticErrc::overflow, "-1 * INT_MIN overflows");

// ---- safe_multiply: underflow (signed) ----

static_assert(safe_multiply(INT_MAX, -2).error() == ArithmeticErrc::underflow, "INT_MAX * -2 underflows");
static_assert(safe_multiply(INT_MIN, 2).error() == ArithmeticErrc::underflow, "INT_MIN * 2 underflows");

// ---- safe_multiply: unsigned overflow ----

static_assert(safe_multiply(UINT_MAX, 2u).error() == ArithmeticErrc::overflow, "UINT_MAX * 2 overflows");

// ---- safe_add: normal cases ----

static_assert(safe_add(10, 20).value() == 30, "10 + 20 = 30");
static_assert(safe_add(-5, 3).value() == -2, "-5 + 3 = -2");
static_assert(safe_add(0, 0).value() == 0, "0 + 0 = 0");

// ---- safe_add: overflow ----

static_assert(safe_add(INT_MAX, 1).error() == ArithmeticErrc::overflow, "INT_MAX + 1 overflows");
static_assert(safe_add(INT_MAX, INT_MAX).error() == ArithmeticErrc::overflow, "INT_MAX + INT_MAX overflows");

// ---- safe_add: underflow ----

static_assert(safe_add(INT_MIN, -1).error() == ArithmeticErrc::underflow, "INT_MIN + (-1) underflows");

// ---- safe_add: unsigned overflow ----

static_assert(safe_add(UINT_MAX, 1u).error() == ArithmeticErrc::overflow, "UINT_MAX + 1 overflows");

// ---- safe_subtract: normal cases ----

static_assert(safe_subtract(30, 10).value() == 20, "30 - 10 = 20");
static_assert(safe_subtract(-5, -3).value() == -2, "-5 - (-3) = -2");

// ---- safe_subtract: overflow ----

static_assert(safe_subtract(INT_MAX, -1).error() == ArithmeticErrc::overflow,
              "INT_MAX - (-1) overflows");

// ---- safe_subtract: underflow ----

static_assert(safe_subtract(INT_MIN, 1).error() == ArithmeticErrc::underflow, "INT_MIN - 1 underflows");

// ---- safe_subtract: unsigned underflow ----

static_assert(safe_subtract(0u, 1u).error() == ArithmeticErrc::underflow, "0u - 1u underflows");

// ---- safe_divide: normal cases ----

static_assert(safe_divide(42, 7).value() == 6, "42 / 7 = 6");
static_assert(safe_divide(-12, 3).value() == -4, "-12 / 3 = -4");

// ---- safe_divide: divide by zero (int) ----

static_assert(safe_divide(42, 0).error() == ArithmeticErrc::divide_by_zero, "42 / 0 = error");

// ---- safe_divide: divide by zero (float) ----

static_assert(safe_divide(1.0, 0.0).error() == ArithmeticErrc::divide_by_zero, "1.0 / 0.0 = error");

// ---- safe_divide: INT_MIN / -1 overflow ----

static_assert(safe_divide(INT_MIN, -1).error() == ArithmeticErrc::overflow, "INT_MIN / -1 overflows");

// ---- safe_to_base: normal int conversion ----

static_assert(
    [] {
        constexpr ScaledUnit<int, LengthTag, std::kilo> km{5};
        constexpr auto result = safe_to_base(km);
        return result.has_value() && result.value().get() == 5000;
    }(),
    "safe_to_base: 5 km = 5000 m");

static_assert(
    [] {
        constexpr ScaledUnit<int, TimeTag, std::ratio<60>> mins{3};
        constexpr auto result = safe_to_base(mins);
        return result.has_value() && result.value().get() == 180;
    }(),
    "safe_to_base: 3 min = 180 s");

// ---- safe_to_base: overflow ----

static_assert(
    [] {
        constexpr ScaledUnit<int, LengthTag, std::kilo> big{3000000};
        constexpr auto result = safe_to_base(big);
        return !result.has_value() && result.error() == ArithmeticErrc::overflow;
    }(),
    "safe_to_base: 3000000 km overflows int");

// ---- safe_scale_cast from base: normal ----

static_assert(
    [] {
        constexpr auto base = unit_t<int, LengthTag>{5000};
        constexpr auto result = safe_scale_cast<ScaledUnit<int, LengthTag, std::kilo>>(base);
        return result.has_value() && result.value().get() == 5;
    }(),
    "safe_scale_cast: 5000 m = 5 km");

// ---- safe_scale_cast from base: truncation ----

static_assert(
    [] {
        constexpr auto base = unit_t<int, LengthTag>{5001};
        constexpr auto result = safe_scale_cast<ScaledUnit<int, LengthTag, std::kilo>>(base);
        return !result.has_value() && result.error() == ArithmeticErrc::truncation;
    }(),
    "safe_scale_cast: 5001 m doesn't convert cleanly to km");

// ---- safe_scale_cast: normal ----

static_assert(
    [] {
        constexpr ScaledUnit<int, TimeTag, std::ratio<3600>> hrs{2};
        constexpr auto result = safe_scale_cast<ScaledUnit<int, TimeTag, std::ratio<60>>>(hrs);
        return result.has_value() && result.value().get() == 120;
    }(),
    "safe_scale_cast: 2 hr = 120 min");

// ---- safe_scale_cast: overflow ----

static_assert(
    [] {
        constexpr ScaledUnit<int, LengthTag, std::kilo> big{2200000};
        constexpr auto result = safe_scale_cast<ScaledUnit<int, LengthTag, std::milli>>(big);
        return !result.has_value();
    }(),
    "safe_scale_cast: 2200000 km -> mm overflows");

// NOLINTEND(readability-magic-numbers,readability-identifier-length)
