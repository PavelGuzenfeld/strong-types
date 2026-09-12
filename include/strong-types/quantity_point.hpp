#pragma once

#include "si.hpp"
#include "si_scaled.hpp"

#include <concepts>
#include <type_traits>
#include <utility>

namespace strong_types
{

// ---- QuantityPoint: affine type (absolute position, not relative displacement) ----

template <typename T, typename Tag, typename Origin = void>
    requires std::is_default_constructible_v<T> && std::copy_constructible<T> && Arithmetic<T>
struct QuantityPoint
{
    using value_type = T;
    using tag_type = Tag;
    using origin_type = Origin;
    using difference_type = unit_t<T, Tag>;

    template <typename U>
        requires std::same_as<std::remove_cvref_t<U>, T>
    constexpr explicit QuantityPoint(U &&value) noexcept : value_(std::forward<U>(value))
    {
    }

    template <typename U>
        requires WideningIntegral<U, T>
    constexpr explicit QuantityPoint(U value) noexcept : value_(static_cast<T>(value))
    {
    }

    template <typename U>
        requires(!std::same_as<std::remove_cvref_t<U>, T> && !WideningIntegral<U, T>)
    explicit QuantityPoint(U && /*unused*/) // NOLINT(cppcoreguidelines-missing-std-forward,google-explicit-constructor)
    {
        static_assert(always_false_v<U>, "narrowing/mismatched construction of QuantityPoint — cast to T first");
    }

    constexpr QuantityPoint() noexcept
        requires std::default_initializable<T>
    = default;

    [[nodiscard]] constexpr T get() const noexcept
    {
        return value_;
    }

    [[nodiscard]] auto operator<=>(const QuantityPoint &) const = default;

private:
    T value_{};
};

// ---- detector ----

template <typename>
inline constexpr bool is_quantity_point_v = false;

template <typename T, typename Tag, typename Origin>
inline constexpr bool is_quantity_point_v<QuantityPoint<T, Tag, Origin>> = true;

// ---- Celsius: a temperature reading; the difference of two readings is a kelvin interval ----

struct CelsiusOrigin
{
};

template <typename T>
using Celsius = QuantityPoint<T, TemperatureTag, CelsiusOrigin>;

// ---- Point - Point (same Origin) → displacement ----

template <typename T, typename Tag, typename Origin>
[[nodiscard]] constexpr unit_t<T, Tag> operator-(const QuantityPoint<T, Tag, Origin> &lhs,
                                                 const QuantityPoint<T, Tag, Origin> &rhs)
{
    return unit_t<T, Tag>{lhs.get() - rhs.get()};
}

// ---- Point + displacement → Point ----

template <typename T, typename Tag, typename Origin>
[[nodiscard]] constexpr QuantityPoint<T, Tag, Origin> operator+(const QuantityPoint<T, Tag, Origin> &point,
                                                                const unit_t<T, Tag> &disp)
{
    return QuantityPoint<T, Tag, Origin>{point.get() + disp.get()};
}

// ---- displacement + Point → Point (commutative) ----

template <typename T, typename Tag, typename Origin>
[[nodiscard]] constexpr QuantityPoint<T, Tag, Origin> operator+(const unit_t<T, Tag> &disp,
                                                                const QuantityPoint<T, Tag, Origin> &point)
{
    return QuantityPoint<T, Tag, Origin>{disp.get() + point.get()};
}

// ---- Point - displacement → Point ----

template <typename T, typename Tag, typename Origin>
[[nodiscard]] constexpr QuantityPoint<T, Tag, Origin> operator-(const QuantityPoint<T, Tag, Origin> &point,
                                                                const unit_t<T, Tag> &disp)
{
    return QuantityPoint<T, Tag, Origin>{point.get() - disp.get()};
}

// ---- Point + ScaledUnit → Point (convert scaled to base, then add) ----

template <typename T, typename Tag, typename Origin, typename R>
    requires ExactScale<T, R>
[[nodiscard]] constexpr QuantityPoint<T, Tag, Origin> operator+(const QuantityPoint<T, Tag, Origin> &point,
                                                                const ScaledUnit<T, Tag, R> &disp)
{
    return point + disp.to_base();
}

// ---- ScaledUnit + Point → Point (commutative) ----

template <typename T, typename Tag, typename Origin, typename R>
    requires ExactScale<T, R>
[[nodiscard]] constexpr QuantityPoint<T, Tag, Origin> operator+(const ScaledUnit<T, Tag, R> &disp,
                                                                const QuantityPoint<T, Tag, Origin> &point)
{
    return disp.to_base() + point;
}

// ---- Point - ScaledUnit → Point ----

template <typename T, typename Tag, typename Origin, typename R>
    requires ExactScale<T, R>
[[nodiscard]] constexpr QuantityPoint<T, Tag, Origin> operator-(const QuantityPoint<T, Tag, Origin> &point,
                                                                const ScaledUnit<T, Tag, R> &disp)
{
    return point - disp.to_base();
}

// NOTE: QuantityPoint + QuantityPoint is intentionally NOT defined.
// Adding two absolute positions is physically meaningless.

} // namespace strong_types
