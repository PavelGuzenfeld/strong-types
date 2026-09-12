#pragma once

#include <compare>
#include <concepts>
#include <type_traits>
#include <utility>

template <typename T>
concept Scalar = std::is_arithmetic_v<std::remove_cvref_t<T>>;

template <typename>
inline constexpr bool always_false_v = false;

namespace strong_types
{
template <typename T>
concept Arithmetic = requires(T lhs, T rhs) {
    { lhs + rhs } -> std::convertible_to<T>;
    { lhs - rhs } -> std::convertible_to<T>;
    { lhs *rhs } -> std::convertible_to<T>;
    { lhs / rhs } -> std::convertible_to<T>;
    { -lhs } -> std::convertible_to<T>;
    { lhs == rhs } -> std::convertible_to<bool>;
    { lhs <=> rhs };
};

// Integral From has no more bits than integral To (int -> uint64_t, short -> long); signedness is not checked.
template <typename From, typename To>
concept WideningIntegral =
    std::is_integral_v<To> && std::is_integral_v<std::remove_cvref_t<From>> &&
    !std::same_as<std::remove_cvref_t<From>, To> && (sizeof(std::remove_cvref_t<From>) <= sizeof(To));

// T needs only the operations actually used; each operator is constrained on its own expression.
template <typename T, typename Tag>
    requires std::is_default_constructible_v<T> && std::copy_constructible<T>
struct Strong
{

    using value_type = T;
    using tag_type = Tag;

    template <typename U>
        requires std::same_as<std::remove_cvref_t<U>, T>
    constexpr explicit Strong(U &&value) noexcept : value_(std::forward<U>(value))
    {
    }

    template <typename U>
        requires WideningIntegral<U, T>
    constexpr explicit Strong(U value) noexcept : value_(static_cast<T>(value))
    {
    }

    template <typename U>
        requires(!std::same_as<std::remove_cvref_t<U>, T> && !WideningIntegral<U, T>)
    explicit Strong(U && /*unused*/) // NOLINT(cppcoreguidelines-missing-std-forward,google-explicit-constructor)
    {
        static_assert(always_false_v<U>,
                      "🚫 invalid construction of strong_types::Strong<T, Tag> with a mismatched type.\n"
                      "💡 try casting explicitly to T first to avoid narrowing conversions or implicit promotions.");
    }

    constexpr explicit(false) Strong() noexcept
        requires std::default_initializable<T>
    = default;

    [[nodiscard]] constexpr T get() const noexcept
    {
        return value_;
    }

    [[nodiscard]] auto operator<=>(const Strong &) const = default;

protected:
    T value_;
};

// ---- scalar division result trait ----
template <typename StrongType, Scalar ScalarType>
struct scalar_division_result
{
    using type = StrongType; // default: keep same type
};

template <typename StrongType, Scalar ScalarType>
using scalar_div_result_t = typename scalar_division_result<StrongType, ScalarType>::type;

// ---- tag-level mapping traits: a pair without a rule has no ::type ----

template <typename LTag, typename RTag>
struct tag_sum_result
{
};
template <typename LTag, typename RTag>
struct tag_difference_result
{
};
template <typename LTag, typename RTag>
struct tag_product_result
{
};
template <typename LTag, typename RTag>
struct tag_quotient_result
{
};

// A void tag is dimensionless: the result is the bare representation
template <typename T, typename Tag>
using tagged_t = std::conditional_t<std::is_void_v<Tag>, T, Strong<T, Tag>>;

template <typename T, typename Rule>
struct tagged_result
{
};

template <typename T, typename Rule>
    requires requires { typename Rule::type; }
struct tagged_result<T, Rule>
{
    using type = tagged_t<T, typename Rule::type>;
};

// Mixed representations follow the usual arithmetic conversions, independent of operand order
template <typename LHS, typename RHS>
using common_rep_t = std::common_type_t<typename LHS::value_type, typename RHS::value_type>;

// Generic fallback: only works for Strong types
template <typename LHS, typename RHS>
    requires requires {
        typename LHS::tag_type;
        typename RHS::tag_type;
        typename common_rep_t<LHS, RHS>;
    }
struct sum_result
    : tagged_result<common_rep_t<LHS, RHS>, tag_sum_result<typename LHS::tag_type, typename RHS::tag_type>>
{
};

template <typename LHS, typename RHS>
    requires requires {
        typename LHS::tag_type;
        typename RHS::tag_type;
        typename common_rep_t<LHS, RHS>;
    }
struct difference_result
    : tagged_result<common_rep_t<LHS, RHS>, tag_difference_result<typename LHS::tag_type, typename RHS::tag_type>>
{
};

template <typename LHS, typename RHS>
    requires requires {
        typename LHS::tag_type;
        typename RHS::tag_type;
        typename common_rep_t<LHS, RHS>;
    }
struct product_result
    : tagged_result<common_rep_t<LHS, RHS>, tag_product_result<typename LHS::tag_type, typename RHS::tag_type>>
{
};

template <typename LHS, typename RHS>
    requires requires {
        typename LHS::tag_type;
        typename RHS::tag_type;
        typename common_rep_t<LHS, RHS>;
    }
struct quotient_result
    : tagged_result<common_rep_t<LHS, RHS>, tag_quotient_result<typename LHS::tag_type, typename RHS::tag_type>>
{
};

// ----- scalar trait specializations ----

template <typename T, typename TAG, Scalar S>
struct product_result<Strong<T, TAG>, Strong<S, void>>
{
    using type = Strong<T, TAG>;
};

template <Scalar S, typename T, typename TAG>
struct product_result<Strong<S, void>, Strong<T, TAG>>
{
    using type = Strong<T, TAG>;
};

template <typename T, typename TAG, Scalar S>
struct quotient_result<Strong<T, TAG>, Strong<S, void>>
{
    using type = Strong<T, TAG>;
};

template <typename T, typename TAG>
struct quotient_result<Strong<T, TAG>, Strong<T, TAG>>
{
    using type = T;
};

// scalar overloads
template <typename T, typename TAG, Scalar S>
struct product_result<Strong<T, TAG>, S>
{
    using type = Strong<T, TAG>;
};

template <typename T, typename TAG, Scalar S>
struct quotient_result<Strong<T, TAG>, S>
{
    using type = Strong<T, TAG>;
};

// ---- Strong-type detector (breaks recursion under clang) ----

template <typename>
inline constexpr bool is_strong_v = false;

template <typename T, typename Tag>
inline constexpr bool is_strong_v<Strong<T, Tag>> = true;

// ---- arithmetic ops (tagged only, Strong operands) ----

template <typename LHS, typename RHS>
    requires is_strong_v<LHS> && requires { typename sum_result<LHS, RHS>::type; } &&
                 requires(const LHS &lhs, const RHS &rhs) { lhs.get() + rhs.get(); }
[[nodiscard]] constexpr auto operator+(const LHS &lhs, const RHS &rhs) -> typename sum_result<LHS, RHS>::type
{
    using R = common_rep_t<LHS, RHS>;
    return typename sum_result<LHS, RHS>::type{static_cast<R>(lhs.get()) + static_cast<R>(rhs.get())};
}

template <typename LHS, typename RHS>
    requires is_strong_v<LHS> && requires { typename difference_result<LHS, RHS>::type; } &&
                 requires(const LHS &lhs, const RHS &rhs) { lhs.get() - rhs.get(); }
[[nodiscard]] constexpr auto operator-(const LHS &lhs, const RHS &rhs) -> typename difference_result<LHS, RHS>::type
{
    using R = common_rep_t<LHS, RHS>;
    return typename difference_result<LHS, RHS>::type{static_cast<R>(lhs.get()) - static_cast<R>(rhs.get())};
}

template <typename LHS, typename RHS>
    requires is_strong_v<LHS> && requires { typename product_result<LHS, RHS>::type; } &&
                 requires(const LHS &lhs, const RHS &rhs) { lhs.get() * rhs.get(); }
[[nodiscard]] constexpr auto operator*(const LHS &lhs, const RHS &rhs) -> typename product_result<LHS, RHS>::type
{
    using R = common_rep_t<LHS, RHS>;
    return typename product_result<LHS, RHS>::type{static_cast<R>(lhs.get()) * static_cast<R>(rhs.get())};
}

template <typename LHS, typename RHS>
    requires is_strong_v<LHS> && requires { typename quotient_result<LHS, RHS>::type; } &&
                 requires(const LHS &lhs, const RHS &rhs) { lhs.get() / rhs.get(); }
[[nodiscard]] constexpr auto operator/(const LHS &lhs, const RHS &rhs) -> typename quotient_result<LHS, RHS>::type
{
    using R = common_rep_t<LHS, RHS>;
    return typename quotient_result<LHS, RHS>::type{static_cast<R>(lhs.get()) / static_cast<R>(rhs.get())};
}

template <typename S>
concept NotStrong = !is_strong_v<std::remove_cvref_t<S>>;

// ---- scalar overloads ----

template <typename T, typename TAG, Scalar S>
    requires NotStrong<S> && requires(const T &val, S scalar) { val *scalar; }
[[nodiscard]] constexpr auto operator*(const Strong<T, TAG> &lhs, S scalar)
{
    if constexpr (std::is_arithmetic_v<T>)
    {
        return Strong<T, TAG>{lhs.get() * static_cast<T>(scalar)};
    }
    else
    {
        return Strong<T, TAG>{lhs.get() * scalar}; // assumes T supports T * Scalar
    }
}

template <typename T, typename TAG, Scalar S>
    requires NotStrong<S> && requires(const T &val, S scalar) { scalar *val; }
[[nodiscard]] constexpr auto operator*(S scalar, const Strong<T, TAG> &rhs)
{
    if constexpr (std::is_arithmetic_v<T>)
    {
        return Strong<T, TAG>{static_cast<T>(scalar) * rhs.get()};
    }
    else
    {
        return Strong<T, TAG>{scalar * rhs.get()}; // assumes Scalar * T works too
    }
}

template <typename T, typename TAG, Scalar S>
    requires NotStrong<S> && requires(const T &val, S scalar) { val / scalar; }
[[nodiscard]] constexpr auto operator/(const Strong<T, TAG> &lhs, S scalar) -> scalar_div_result_t<Strong<T, TAG>, S>
{
    if constexpr (std::is_arithmetic_v<T>)
    {
        return scalar_div_result_t<Strong<T, TAG>, S>{lhs.get() / static_cast<T>(scalar)};
    }
    else
    {
        return scalar_div_result_t<Strong<T, TAG>, S>{lhs.get() / scalar};
    }
}

template <typename T, typename TAG, Scalar S>
    requires NotStrong<S> && requires(const T &val, S scalar) { scalar / val; }
[[nodiscard]] constexpr auto operator/(S scalar, const Strong<T, TAG> &rhs)
{
    using result_t = typename quotient_result<Strong<S, void>, Strong<T, TAG>>::type;
    using R = std::common_type_t<S, T>;
    return result_t{static_cast<R>(scalar) / static_cast<R>(rhs.get())};
}

// ---- compound assignment ----

template <typename T, typename TAG>
    requires requires(const Strong<T, TAG> &val) { val + val; }
constexpr Strong<T, TAG> &operator+=(Strong<T, TAG> &lhs, const Strong<T, TAG> &rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

template <typename T, typename TAG>
    requires requires(const Strong<T, TAG> &val) { val - val; }
constexpr Strong<T, TAG> &operator-=(Strong<T, TAG> &lhs, const Strong<T, TAG> &rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

template <typename T, typename TAG, Scalar S>
    requires requires(const Strong<T, TAG> &val, S scalar) { val *scalar; }
constexpr Strong<T, TAG> &operator*=(Strong<T, TAG> &lhs, S scalar)
{
    lhs = lhs * scalar;
    return lhs;
}

// compound assignment for scalar division, only if scalar_div_result matches Strong<T, TAG>
template <typename T, typename TAG, Scalar S>
    requires std::is_same_v<scalar_div_result_t<Strong<T, TAG>, S>, Strong<T, TAG>> &&
             requires(const Strong<T, TAG> &val, S scalar) { val / scalar; }
constexpr Strong<T, TAG> &operator/=(Strong<T, TAG> &lhs, S scalar)
{
    lhs = lhs / scalar;
    return lhs;
}

// ---- unary ----

template <typename T, typename TAG>
    requires requires(const T &val) { -val; }
[[nodiscard]] constexpr Strong<T, TAG> operator-(const Strong<T, TAG> &val)
{
    return Strong<T, TAG>(-val.get());
}

template <typename T, typename TAG>
    requires requires(const T &val) {
        { val == val } -> std::convertible_to<bool>;
    }
[[nodiscard]] constexpr bool operator==(const Strong<T, TAG> &lhs, const Strong<T, TAG> &rhs)
{
    return lhs.get() == rhs.get();
}

} // namespace strong_types
