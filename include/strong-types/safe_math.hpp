#pragma once

#include "si.hpp"
#include "si_scaled.hpp"

#include <cmath>
#include <concepts>
#include <expected>
#include <limits>
#include <type_traits>

namespace strong_types
{

enum class ArithmeticErrc
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
        return val >= 0 ? static_cast<U>(val) : static_cast<U>(~val) + U{1};
    }
    else
    {
        return val;
    }
}

// ---- safe_multiply ----

template <std::integral T>
[[nodiscard]] constexpr auto safe_multiply(T a, T b) -> std::expected<T, ArithmeticErrc>
{
    if (a == 0 || b == 0)
    {
        return T{0};
    }

    using U = std::make_unsigned_t<T>;
    constexpr auto max_val = std::numeric_limits<T>::max();
    constexpr auto min_val = std::numeric_limits<T>::min();

    if constexpr (std::is_signed_v<T>)
    {
        auto abs_a = unsigned_abs(a);
        auto abs_b = unsigned_abs(b);
        bool result_negative = (a > 0) != (b > 0);

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
            return static_cast<T>(-static_cast<T>(unsigned_result - U{1}) - T{1});
        }
        return static_cast<T>(unsigned_result);
    }
    else
    {
        // Unsigned: only overflow possible
        if (a > max_val / b)
        {
            return std::unexpected{ArithmeticErrc::overflow};
        }
        return static_cast<T>(a * b);
    }
}

// ---- safe_add ----

template <std::integral T>
[[nodiscard]] constexpr auto safe_add(T a, T b) -> std::expected<T, ArithmeticErrc>
{
    constexpr auto max_val = std::numeric_limits<T>::max();
    constexpr auto min_val = std::numeric_limits<T>::min();

    if constexpr (std::is_signed_v<T>)
    {
        if (b > 0 && a > max_val - b)
        {
            return std::unexpected{ArithmeticErrc::overflow};
        }
        if (b < 0 && a < min_val - b)
        {
            return std::unexpected{ArithmeticErrc::underflow};
        }
    }
    else
    {
        if (a > max_val - b)
        {
            return std::unexpected{ArithmeticErrc::overflow};
        }
    }

    return static_cast<T>(a + b);
}

// ---- safe_subtract ----

template <std::integral T>
[[nodiscard]] constexpr auto safe_subtract(T a, T b) -> std::expected<T, ArithmeticErrc>
{
    constexpr auto max_val = std::numeric_limits<T>::max();
    constexpr auto min_val = std::numeric_limits<T>::min();

    if constexpr (std::is_signed_v<T>)
    {
        if (b < 0 && a > max_val + b)
        {
            return std::unexpected{ArithmeticErrc::overflow};
        }
        if (b > 0 && a < min_val + b)
        {
            return std::unexpected{ArithmeticErrc::underflow};
        }
    }
    else
    {
        if (a < b)
        {
            return std::unexpected{ArithmeticErrc::underflow};
        }
    }

    return static_cast<T>(a - b);
}

// ---- safe_divide ----

template <typename T>
[[nodiscard]] constexpr auto safe_divide(T a, T b) -> std::expected<T, ArithmeticErrc>
{
    if constexpr (std::is_floating_point_v<T>)
    {
        if (b == T{0})
        {
            return std::unexpected{ArithmeticErrc::divide_by_zero};
        }
        T result = a / b;
        if (std::isinf(result) || std::isnan(result))
        {
            return std::unexpected{ArithmeticErrc::overflow};
        }
        return result;
    }
    else
    {
        static_assert(std::integral<T>);
        if (b == T{0})
        {
            return std::unexpected{ArithmeticErrc::divide_by_zero};
        }
        if constexpr (std::is_signed_v<T>)
        {
            if (a == std::numeric_limits<T>::min() && b == T{-1})
            {
                return std::unexpected{ArithmeticErrc::overflow};
            }
        }
        return static_cast<T>(a / b);
    }
}

// ---- safe_to_base: ScaledUnit<int> → unit_t<int> with overflow check ----

template <typename T, typename Tag, typename Ratio>
    requires std::integral<T>
[[nodiscard]] constexpr auto safe_to_base(ScaledUnit<T, Tag, Ratio> val)
    -> std::expected<unit_t<T, Tag>, ArithmeticErrc>
{
    // to_base = value * num / den
    auto mul = safe_multiply(val.get(), static_cast<T>(Ratio::num));
    if (!mul)
    {
        return std::unexpected{mul.error()};
    }
    auto div = safe_divide(*mul, static_cast<T>(Ratio::den));
    if (!div)
    {
        return std::unexpected{div.error()};
    }
    return unit_t<T, Tag>{*div};
}

// ---- safe_from_base: unit_t<int> → ScaledUnit<int> with overflow/truncation check ----

template <typename TargetScaled, typename T, typename Tag>
    requires is_scaled_v<TargetScaled> && std::is_same_v<typename TargetScaled::tag_type, Tag> &&
                 std::integral<typename TargetScaled::value_type>
[[nodiscard]] constexpr auto safe_from_base(unit_t<T, Tag> base)
    -> std::expected<TargetScaled, ArithmeticErrc>
{
    using TargetT = typename TargetScaled::value_type;
    using R = typename TargetScaled::ratio_type;

    // from_base = base * den / num
    auto mul = safe_multiply(static_cast<TargetT>(base.get()), static_cast<TargetT>(R::den));
    if (!mul)
    {
        return std::unexpected{mul.error()};
    }

    // Check for truncation: if (*mul % num != 0), we lose precision
    auto num = static_cast<TargetT>(R::num);
    if ((*mul % num) != TargetT{0})
    {
        return std::unexpected{ArithmeticErrc::truncation};
    }

    auto div = safe_divide(*mul, num);
    if (!div)
    {
        return std::unexpected{div.error()};
    }
    return TargetScaled{*div};
}

// ---- safe_scale_cast: ScaledUnit<int> → ScaledUnit<int> with overflow check ----

template <typename TargetScaled, typename T, typename Tag, typename R>
    requires is_scaled_v<TargetScaled> && std::is_same_v<typename TargetScaled::tag_type, Tag> &&
                 std::integral<typename TargetScaled::value_type>
[[nodiscard]] constexpr auto safe_scale_cast(ScaledUnit<T, Tag, R> from)
    -> std::expected<TargetScaled, ArithmeticErrc>
{
    using TargetR = typename TargetScaled::ratio_type;
    using F = std::ratio_divide<R, TargetR>;
    using TargetT = typename TargetScaled::value_type;

    auto mul = safe_multiply(static_cast<TargetT>(from.get()), static_cast<TargetT>(F::num));
    if (!mul)
    {
        return std::unexpected{mul.error()};
    }

    auto den = static_cast<TargetT>(F::den);
    if ((*mul % den) != TargetT{0})
    {
        return std::unexpected{ArithmeticErrc::truncation};
    }

    auto div = safe_divide(*mul, den);
    if (!div)
    {
        return std::unexpected{div.error()};
    }
    return TargetScaled{*div};
}

} // namespace strong_types
