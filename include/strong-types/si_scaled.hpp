#pragma once

#include "si.hpp"
#include <ratio>

namespace strong_types
{

// ---- ScaledUnit detector (must be declared before use in constraints) ----

template <typename>
inline constexpr bool is_scaled_v = false;

template <typename S>
concept NotScaled = !is_scaled_v<std::remove_cvref_t<S>>;

// ---- ScaledUnit class ----

template <typename T, typename Tag, typename Ratio>
    requires std::is_default_constructible_v<T> && Arithmetic<T>
struct ScaledUnit
{
    using value_type = T;
    using tag_type = Tag;
    using ratio_type = Ratio;

    template <typename U>
        requires std::same_as<std::remove_cvref_t<U>, T>
    constexpr explicit ScaledUnit(U &&val) noexcept : value_(std::forward<U>(val))
    {
    }

    template <typename U>
        requires(!std::same_as<std::remove_cvref_t<U>, T>)
    explicit ScaledUnit(U && /*unused*/) // NOLINT(cppcoreguidelines-missing-std-forward,google-explicit-constructor)
    {
        static_assert(always_false_v<U>,
                      "narrowing/mismatched construction of ScaledUnit — cast to T first");
    }

    constexpr ScaledUnit() noexcept = default;

    [[nodiscard]] constexpr T get() const noexcept
    {
        return value_;
    }

    /// Convert to base unit_t<T, Tag>
    [[nodiscard]] constexpr unit_t<T, Tag> to_base() const noexcept
    {
        return unit_t<T, Tag>{value_ * static_cast<T>(Ratio::num) / static_cast<T>(Ratio::den)};
    }

    /// Convert to another scale
    template <typename TargetRatio>
    [[nodiscard]] constexpr ScaledUnit<T, Tag, TargetRatio> in() const noexcept
    {
        using F = std::ratio_divide<Ratio, TargetRatio>;
        return ScaledUnit<T, Tag, TargetRatio>{value_ * static_cast<T>(F::num) / static_cast<T>(F::den)};
    }

    [[nodiscard]] auto operator<=>(const ScaledUnit &) const = default;

private:
    T value_{};
};

// ---- detector specialization ----

template <typename T, typename Tag, typename R>
inline constexpr bool is_scaled_v<ScaledUnit<T, Tag, R>> = true;

// ---- same-tag helper concept ----

template <typename A, typename B>
concept SameTagScaled = is_scaled_v<A> && is_scaled_v<B> && std::is_same_v<typename A::tag_type, typename B::tag_type>;

// ---- from_base: base unit → ScaledUnit ----

template <typename TargetScaled, typename T, typename Tag>
    requires is_scaled_v<TargetScaled> && std::is_same_v<typename TargetScaled::tag_type, Tag>
[[nodiscard]] constexpr TargetScaled from_base(unit_t<T, Tag> base) noexcept
{
    using TargetT = typename TargetScaled::value_type;
    using R = typename TargetScaled::ratio_type;
    return TargetScaled{static_cast<TargetT>(
        base.get() * static_cast<T>(R::den) / static_cast<T>(R::num))};
}

// ---- checked_cast: ScaledUnit → ScaledUnit (explicit scale conversion) ----

template <typename TargetScaled, typename T, typename Tag, typename R>
    requires is_scaled_v<TargetScaled> && std::is_same_v<typename TargetScaled::tag_type, Tag>
[[nodiscard]] constexpr TargetScaled checked_cast(ScaledUnit<T, Tag, R> from) noexcept
{
    using TargetT = typename TargetScaled::value_type;
    using TargetR = typename TargetScaled::ratio_type;
    using F = std::ratio_divide<R, TargetR>;
    return TargetScaled{static_cast<TargetT>(
        from.get() * static_cast<T>(F::num) / static_cast<T>(F::den))};
}

// ---- unary negate ----

template <typename T, typename Tag, typename R>
[[nodiscard]] constexpr ScaledUnit<T, Tag, R> operator-(const ScaledUnit<T, Tag, R> &val)
{
    return ScaledUnit<T, Tag, R>{-val.get()};
}

// ---- same-scale add/sub (same tag, same ratio) ----

template <typename T, typename Tag, typename R>
[[nodiscard]] constexpr ScaledUnit<T, Tag, R> operator+(const ScaledUnit<T, Tag, R> &lhs,
                                                        const ScaledUnit<T, Tag, R> &rhs)
{
    return ScaledUnit<T, Tag, R>{lhs.get() + rhs.get()};
}

template <typename T, typename Tag, typename R>
[[nodiscard]] constexpr ScaledUnit<T, Tag, R> operator-(const ScaledUnit<T, Tag, R> &lhs,
                                                        const ScaledUnit<T, Tag, R> &rhs)
{
    return ScaledUnit<T, Tag, R>{lhs.get() - rhs.get()};
}

// ---- cross-scale add/sub (same tag, different ratio -> base) ----

template <typename LHS, typename RHS>
    requires SameTagScaled<LHS, RHS> && (!std::is_same_v<typename LHS::ratio_type, typename RHS::ratio_type>)
[[nodiscard]] constexpr auto operator+(const LHS &lhs, const RHS &rhs)
{
    return lhs.to_base() + rhs.to_base();
}

template <typename LHS, typename RHS>
    requires SameTagScaled<LHS, RHS> && (!std::is_same_v<typename LHS::ratio_type, typename RHS::ratio_type>)
[[nodiscard]] constexpr auto operator-(const LHS &lhs, const RHS &rhs)
{
    return lhs.to_base() - rhs.to_base();
}

// ---- scaled +/- base unit_t (same tag -> base) ----

template <typename T, typename Tag, typename R>
[[nodiscard]] constexpr unit_t<T, Tag> operator+(const ScaledUnit<T, Tag, R> &lhs, const unit_t<T, Tag> &rhs)
{
    return lhs.to_base() + rhs;
}

template <typename T, typename Tag, typename R>
[[nodiscard]] constexpr unit_t<T, Tag> operator+(const unit_t<T, Tag> &lhs, const ScaledUnit<T, Tag, R> &rhs)
{
    return lhs + rhs.to_base();
}

template <typename T, typename Tag, typename R>
[[nodiscard]] constexpr unit_t<T, Tag> operator-(const ScaledUnit<T, Tag, R> &lhs, const unit_t<T, Tag> &rhs)
{
    return lhs.to_base() - rhs;
}

template <typename T, typename Tag, typename R>
[[nodiscard]] constexpr unit_t<T, Tag> operator-(const unit_t<T, Tag> &lhs, const ScaledUnit<T, Tag, R> &rhs)
{
    return lhs - rhs.to_base();
}

// ---- scalar multiply/divide (preserve scale) ----

template <typename T, typename Tag, typename R, Scalar S>
    requires NotStrong<S> && NotScaled<S>
[[nodiscard]] constexpr auto operator*(const ScaledUnit<T, Tag, R> &lhs, S scalar)
{
    return ScaledUnit<T, Tag, R>{lhs.get() * static_cast<T>(scalar)};
}

template <typename T, typename Tag, typename R, Scalar S>
    requires NotStrong<S> && NotScaled<S>
[[nodiscard]] constexpr auto operator*(S scalar, const ScaledUnit<T, Tag, R> &rhs)
{
    return ScaledUnit<T, Tag, R>{static_cast<T>(scalar) * rhs.get()};
}

template <typename T, typename Tag, typename R, Scalar S>
    requires NotStrong<S> && NotScaled<S>
[[nodiscard]] constexpr auto operator/(const ScaledUnit<T, Tag, R> &lhs, S scalar)
{
    return ScaledUnit<T, Tag, R>{lhs.get() / static_cast<T>(scalar)};
}

// ---- scaled x scaled multiply (delegates to base Strong ops) ----

template <typename T, typename LTag, typename LR, typename RTag, typename RR>
[[nodiscard]] constexpr auto operator*(const ScaledUnit<T, LTag, LR> &lhs, const ScaledUnit<T, RTag, RR> &rhs)
{
    return lhs.to_base() * rhs.to_base();
}

// ---- scaled / scaled divide (delegates to base Strong ops) ----

template <typename T, typename LTag, typename LR, typename RTag, typename RR>
[[nodiscard]] constexpr auto operator/(const ScaledUnit<T, LTag, LR> &lhs, const ScaledUnit<T, RTag, RR> &rhs)
{
    return lhs.to_base() / rhs.to_base();
}

// ---- scaled x base unit_t (delegates to base) ----

template <typename T, typename LTag, typename R, typename RTag>
[[nodiscard]] constexpr auto operator*(const ScaledUnit<T, LTag, R> &lhs, const unit_t<T, RTag> &rhs)
{
    return lhs.to_base() * rhs;
}

template <typename T, typename LTag, typename RTag, typename R>
[[nodiscard]] constexpr auto operator*(const unit_t<T, LTag> &lhs, const ScaledUnit<T, RTag, R> &rhs)
{
    return lhs * rhs.to_base();
}

template <typename T, typename LTag, typename R, typename RTag>
[[nodiscard]] constexpr auto operator/(const ScaledUnit<T, LTag, R> &lhs, const unit_t<T, RTag> &rhs)
{
    return lhs.to_base() / rhs;
}

template <typename T, typename LTag, typename RTag, typename R>
[[nodiscard]] constexpr auto operator/(const unit_t<T, LTag> &lhs, const ScaledUnit<T, RTag, R> &rhs)
{
    return lhs / rhs.to_base();
}

// ---- cross-scale comparison (same tag, different ratio) ----

template <typename LHS, typename RHS>
    requires SameTagScaled<LHS, RHS> && (!std::is_same_v<typename LHS::ratio_type, typename RHS::ratio_type>)
[[nodiscard]] constexpr bool operator==(const LHS &lhs, const RHS &rhs)
{
    return lhs.to_base().get() == rhs.to_base().get();
}

template <typename LHS, typename RHS>
    requires SameTagScaled<LHS, RHS> && (!std::is_same_v<typename LHS::ratio_type, typename RHS::ratio_type>)
[[nodiscard]] constexpr auto operator<=>(const LHS &lhs, const RHS &rhs)
{
    return lhs.to_base().get() <=> rhs.to_base().get();
}

// ---- ScaledUnit vs unit_t comparison ----

template <typename T, typename Tag, typename R>
[[nodiscard]] constexpr bool operator==(const ScaledUnit<T, Tag, R> &lhs, const unit_t<T, Tag> &rhs)
{
    return lhs.to_base().get() == rhs.get();
}

template <typename T, typename Tag, typename R>
[[nodiscard]] constexpr auto operator<=>(const ScaledUnit<T, Tag, R> &lhs, const unit_t<T, Tag> &rhs)
{
    return lhs.to_base().get() <=> rhs.get();
}

// ---- type aliases ----

// Length
template <typename T>
using Micrometers = ScaledUnit<T, LengthTag, std::micro>;
template <typename T>
using Millimeters = ScaledUnit<T, LengthTag, std::milli>;
template <typename T>
using Centimeters = ScaledUnit<T, LengthTag, std::centi>;
template <typename T>
using Kilometers = ScaledUnit<T, LengthTag, std::kilo>;

// Time
template <typename T>
using Nanoseconds = ScaledUnit<T, TimeTag, std::nano>;
template <typename T>
using Microseconds = ScaledUnit<T, TimeTag, std::micro>;
template <typename T>
using Milliseconds = ScaledUnit<T, TimeTag, std::milli>;
template <typename T>
using Minutes = ScaledUnit<T, TimeTag, std::ratio<60>>; // NOLINT(readability-magic-numbers)
template <typename T>
using Hours = ScaledUnit<T, TimeTag, std::ratio<3600>>; // NOLINT(readability-magic-numbers)
template <typename T>
using Days = ScaledUnit<T, TimeTag, std::ratio<86400>>; // NOLINT(readability-magic-numbers)
template <typename T>
using Weeks = ScaledUnit<T, TimeTag, std::ratio<604800>>; // NOLINT(readability-magic-numbers)

// Mass (base = kilograms)
template <typename T>
using Milligrams = ScaledUnit<T, MassTag, std::ratio<1, 1000000>>; // NOLINT(readability-magic-numbers)
template <typename T>
using Grams = ScaledUnit<T, MassTag, std::ratio<1, 1000>>; // NOLINT(readability-magic-numbers)
template <typename T>
using Tons = ScaledUnit<T, MassTag, std::kilo>;

// Speed
template <typename T>
using KilometersPerHour = ScaledUnit<T, SpeedTag, std::ratio<5, 18>>; // NOLINT(readability-magic-numbers)

} // namespace strong_types
