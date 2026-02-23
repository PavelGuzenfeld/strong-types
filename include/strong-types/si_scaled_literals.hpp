#pragma once

#include "si_scaled.hpp"

namespace strong_types::si_scaled_literals
{

// NOLINTBEGIN(cppcoreguidelines-macro-usage,bugprone-macro-parentheses) — macro required for UDL token-pasting
#define STRONG_TYPES_DEFINE_SCALED_UDL(suffix, Type)                                                                   \
    [[nodiscard]] constexpr Type operator""_##suffix(long double val)                                                  \
    {                                                                                                                  \
        return Type{static_cast<double>(val)};                                                                         \
    }                                                                                                                  \
    [[nodiscard]] constexpr Type operator""_##suffix(unsigned long long val)                                           \
    {                                                                                                                  \
        return Type{static_cast<double>(val)};                                                                         \
    }

// Length
STRONG_TYPES_DEFINE_SCALED_UDL(km, Kilometers<double>)
STRONG_TYPES_DEFINE_SCALED_UDL(cm, Centimeters<double>)
STRONG_TYPES_DEFINE_SCALED_UDL(mm, Millimeters<double>)

// Time
STRONG_TYPES_DEFINE_SCALED_UDL(min, Minutes<double>)
STRONG_TYPES_DEFINE_SCALED_UDL(hr, Hours<double>)
STRONG_TYPES_DEFINE_SCALED_UDL(ms, Milliseconds<double>)
STRONG_TYPES_DEFINE_SCALED_UDL(us, Microseconds<double>)
STRONG_TYPES_DEFINE_SCALED_UDL(ns, Nanoseconds<double>)

// Mass
STRONG_TYPES_DEFINE_SCALED_UDL(g, Grams<double>)
STRONG_TYPES_DEFINE_SCALED_UDL(mg, Milligrams<double>)

#undef STRONG_TYPES_DEFINE_SCALED_UDL
// NOLINTEND(cppcoreguidelines-macro-usage,bugprone-macro-parentheses)

} // namespace strong_types::si_scaled_literals
