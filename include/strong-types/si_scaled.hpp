#pragma once

#include "safe_math.hpp"
#include "si.hpp"

#include <ratio>
#include <utility>

namespace strong_types
{

// ---- ScaledUnit detector (must be declared before use in constraints) ----

template <typename>
inline constexpr bool is_scaled_v = false;

template <typename S>
concept NotScaled = !is_scaled_v<std::remove_cvref_t<S>>;

// ---- exact scale conversion ----

// Integral reps convert only by an integer factor that fits T; anything else needs safe_to_base/safe_scale_cast.
template <typename T, typename Ratio>
concept ExactScale = std::floating_point<T> || (Ratio::den == 1 && std::in_range<T>(Ratio::num));

// Integral overflow fails constant evaluation, or terminates at runtime.
template <typename Ratio, typename T>
    requires ExactScale<T, Ratio>
[[nodiscard]] constexpr T rescale(T value) noexcept
{
    if constexpr (std::integral<T>)
    {
        return safe_multiply(value, static_cast<T>(Ratio::num)).value();
    }
    else
    {
        return value * static_cast<T>(Ratio::num) / static_cast<T>(Ratio::den);
    }
}

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
        static_assert(always_false_v<U>, "narrowing/mismatched construction of ScaledUnit — cast to T first");
    }

    constexpr ScaledUnit() noexcept = default;

    [[nodiscard]] constexpr T get() const noexcept
    {
        return value_;
    }

    [[nodiscard]] constexpr unit_t<T, Tag> to_base() const noexcept
        requires ExactScale<T, Ratio>
    {
        return unit_t<T, Tag>{rescale<Ratio>(value_)};
    }

    template <typename TargetRatio>
        requires ExactScale<T, std::ratio_divide<Ratio, TargetRatio>>
    [[nodiscard]] constexpr ScaledUnit<T, Tag, TargetRatio> in() const noexcept
    {
        return ScaledUnit<T, Tag, TargetRatio>{rescale<std::ratio_divide<Ratio, TargetRatio>>(value_)};
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

// ---- scale_cast: base unit_t → ScaledUnit (implicitly ratio<1>) ----

template <typename TargetScaled, typename T, typename Tag>
    requires is_scaled_v<TargetScaled> && std::is_same_v<typename TargetScaled::tag_type, Tag> &&
             ExactScale<T, std::ratio_divide<std::ratio<1>, typename TargetScaled::ratio_type>>
[[nodiscard]] constexpr TargetScaled scale_cast(unit_t<T, Tag> base) noexcept
{
    using TargetT = typename TargetScaled::value_type;
    using F = std::ratio_divide<std::ratio<1>, typename TargetScaled::ratio_type>;
    return TargetScaled{static_cast<TargetT>(rescale<F>(base.get()))};
}

// ---- scale_cast: ScaledUnit → ScaledUnit (explicit scale conversion) ----

template <typename TargetScaled, typename T, typename Tag, typename R>
    requires is_scaled_v<TargetScaled> && std::is_same_v<typename TargetScaled::tag_type, Tag> &&
             ExactScale<T, std::ratio_divide<R, typename TargetScaled::ratio_type>>
[[nodiscard]] constexpr TargetScaled scale_cast(ScaledUnit<T, Tag, R> from) noexcept
{
    using TargetT = typename TargetScaled::value_type;
    using F = std::ratio_divide<R, typename TargetScaled::ratio_type>;
    return TargetScaled{static_cast<TargetT>(rescale<F>(from.get()))};
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

// ---- safe_to_base: ScaledUnit<int> → unit_t<int> with overflow and truncation check ----

template <typename T, typename Tag, typename Ratio>
    requires std::integral<T>
[[nodiscard]] constexpr auto safe_to_base(ScaledUnit<T, Tag, Ratio> val)
    -> std::expected<unit_t<T, Tag>, ArithmeticErrc>
{
    auto mul = safe_multiply(val.get(), static_cast<T>(Ratio::num));
    if (!mul)
    {
        return std::unexpected{mul.error()};
    }

    auto den = static_cast<T>(Ratio::den);
    if ((*mul % den) != T{0})
    {
        return std::unexpected{ArithmeticErrc::truncation};
    }

    auto div = safe_divide(*mul, den);
    if (!div)
    {
        return std::unexpected{div.error()};
    }
    return unit_t<T, Tag>{*div};
}

// ---- safe_scale_cast: base unit_t<int> → ScaledUnit<int> (implicitly ratio<1>) ----

template <typename TargetScaled, typename T, typename Tag>
    requires is_scaled_v<TargetScaled> && std::is_same_v<typename TargetScaled::tag_type, Tag> &&
                 std::integral<typename TargetScaled::value_type>
[[nodiscard]] constexpr auto safe_scale_cast(unit_t<T, Tag> base) -> std::expected<TargetScaled, ArithmeticErrc>
{
    using TargetT = typename TargetScaled::value_type;
    using R = typename TargetScaled::ratio_type;

    auto mul = safe_multiply(static_cast<TargetT>(base.get()), static_cast<TargetT>(R::den));
    if (!mul)
    {
        return std::unexpected{mul.error()};
    }

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
[[nodiscard]] constexpr auto safe_scale_cast(ScaledUnit<T, Tag, R> from) -> std::expected<TargetScaled, ArithmeticErrc>
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
