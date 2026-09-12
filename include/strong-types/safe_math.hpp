#pragma once

#include <cmath>
#include <concepts>
#include <cstdint>
#include <expected>
#include <limits>
#include <type_traits>

namespace strong_types
{

enum class ArithmeticErrc : std::uint8_t
{
    overflow,
    underflow,
    divide_by_zero,
    truncation
};

// ---- helpers ----

// UB-safe absolute value: works for INT_MIN (where -a would overflow)
template <std::integral T>
[[nodiscard]] constexpr auto unsigned_abs(T val) -> std::make_unsigned_t<T>
{
    using U = std::make_unsigned_t<T>;
    if constexpr (std::is_signed_v<T>)
    {
        // Two's complement: -val == ~val + 1, computed in unsigned to avoid UB
        return val >= 0 ? static_cast<U>(val) : static_cast<U>(~val) + U{1}; // NOLINT(readability-magic-numbers)
    }
    else
    {
        return val;
    }
}

// ---- safe_multiply ----

template <std::integral T>
[[nodiscard]] constexpr auto safe_multiply(T lhs, T rhs) -> std::expected<T, ArithmeticErrc>
{
    if (lhs == 0 || rhs == 0)
    {
        return T{0};
    }

    using U = std::make_unsigned_t<T>;
    constexpr auto max_val = std::numeric_limits<T>::max();
    constexpr auto min_val = std::numeric_limits<T>::min();

    if constexpr (std::is_signed_v<T>)
    {
        auto abs_a = unsigned_abs(lhs);
        auto abs_b = unsigned_abs(rhs);
        bool result_negative = (lhs > 0) != (rhs > 0);

        if (result_negative)
        {
            // Result must fit in [min_val, -1] → abs(result) <= -min_val == unsigned_abs(min_val)
            auto limit = unsigned_abs(min_val);
            if (abs_a > limit / abs_b)
            {
                return std::unexpected{ArithmeticErrc::underflow};
            }
        }
        else
        {
            // Result must fit in [1, max_val]
            if (abs_a > static_cast<U>(max_val) / abs_b)
            {
                return std::unexpected{ArithmeticErrc::overflow};
            }
        }

        // Compute in unsigned, then convert back (safe: we verified it fits)
        auto unsigned_result = abs_a * abs_b;
        if (result_negative)
        {
            return static_cast<T>(-static_cast<T>(unsigned_result - U{1}) - T{1}); // NOLINT(readability-magic-numbers)
        }
        return static_cast<T>(unsigned_result);
    }
    else
    {
        // Unsigned: only overflow possible
        if (lhs > max_val / rhs)
        {
            return std::unexpected{ArithmeticErrc::overflow};
        }
        return static_cast<T>(lhs * rhs);
    }
}

// ---- safe_add ----

template <std::integral T>
[[nodiscard]] constexpr auto safe_add(T lhs, T rhs) -> std::expected<T, ArithmeticErrc>
{
    constexpr auto max_val = std::numeric_limits<T>::max();
    constexpr auto min_val = std::numeric_limits<T>::min();

    if constexpr (std::is_signed_v<T>)
    {
        if (rhs > 0 && lhs > max_val - rhs)
        {
            return std::unexpected{ArithmeticErrc::overflow};
        }
        if (rhs < 0 && lhs < min_val - rhs)
        {
            return std::unexpected{ArithmeticErrc::underflow};
        }
    }
    else
    {
        if (lhs > max_val - rhs)
        {
            return std::unexpected{ArithmeticErrc::overflow};
        }
    }

    return static_cast<T>(lhs + rhs);
}

// ---- safe_subtract ----

template <std::integral T>
[[nodiscard]] constexpr auto safe_subtract(T lhs, T rhs) -> std::expected<T, ArithmeticErrc>
{
    constexpr auto max_val = std::numeric_limits<T>::max();
    constexpr auto min_val = std::numeric_limits<T>::min();

    if constexpr (std::is_signed_v<T>)
    {
        if (rhs < 0 && lhs > max_val + rhs)
        {
            return std::unexpected{ArithmeticErrc::overflow};
        }
        if (rhs > 0 && lhs < min_val + rhs)
        {
            return std::unexpected{ArithmeticErrc::underflow};
        }
    }
    else
    {
        if (lhs < rhs)
        {
            return std::unexpected{ArithmeticErrc::underflow};
        }
    }

    return static_cast<T>(lhs - rhs);
}

// ---- safe_divide ----

template <typename T>
[[nodiscard]] constexpr auto safe_divide(T lhs, T rhs) -> std::expected<T, ArithmeticErrc>
{
    if constexpr (std::is_floating_point_v<T>)
    {
        if (rhs == T{0})
        {
            return std::unexpected{ArithmeticErrc::divide_by_zero};
        }
        T result = lhs / rhs;
        if (std::isinf(result) || std::isnan(result))
        {
            return std::unexpected{ArithmeticErrc::overflow};
        }
        return result;
    }
    else
    {
        static_assert(std::integral<T>);
        if (rhs == T{0})
        {
            return std::unexpected{ArithmeticErrc::divide_by_zero};
        }
        if constexpr (std::is_signed_v<T>)
        {
            if (lhs == std::numeric_limits<T>::min() && rhs == T{-1}) // NOLINT(readability-magic-numbers)
            {
                return std::unexpected{ArithmeticErrc::overflow};
            }
        }
        return static_cast<T>(lhs / rhs);
    }
}

} // namespace strong_types
