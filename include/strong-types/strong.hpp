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
    concept Arithmetic = requires(T a, T b) {
        { a + b } -> std::convertible_to<T>;
        { a - b } -> std::convertible_to<T>;
        { a * b } -> std::convertible_to<T>;
        { a / b } -> std::convertible_to<T>;
        { -a } -> std::convertible_to<T>;
        { a == b } -> std::convertible_to<bool>;
        { a <=> b };
    };

    template <typename T, typename Tag>
    struct Strong
    {
        static_assert(std::is_default_constructible_v<T>, "need default ctor, genius");
        static_assert(std::copy_constructible<T>, "copy constructor required for strong types");
        static_assert(Arithmetic<T>, "your struct is dumb. go teach it math first.");

        using value_type = T;
        using tag_type = Tag;

        template <typename U>
            requires std::same_as<std::remove_cvref_t<U>, T>
        constexpr explicit Strong(U &&value) noexcept
            : value_(std::forward<U>(value))
        {
        }

        template <typename U>
            requires(!std::same_as<std::remove_cvref_t<U>, T>)
        Strong(U &&)
        {
            static_assert(always_false_v<U>,
                          "🚫 invalid construction of strong_types::Strong<T, Tag> with a mismatched type.\n"
                          "💡 try casting explicitly to T first to avoid narrowing conversions or implicit promotions.");
        }

        constexpr explicit(false) Strong() noexcept
            requires std::default_initializable<T>
        = default;

        [[nodiscard]] constexpr T get() const noexcept { return value_; }

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

    // ---- tag-level mapping traits ----

    template <typename LTag, typename RTag>
    struct tag_sum_result;
    template <typename LTag, typename RTag>
    struct tag_difference_result;
    template <typename LTag, typename RTag>
    struct tag_product_result;
    template <typename LTag, typename RTag>
    struct tag_quotient_result;

    // Generic fallback: only works for Strong types
    template <typename LHS, typename RHS>
        requires requires { typename LHS::tag_type; typename RHS::tag_type; }
    struct sum_result
    {
        using type = Strong<typename LHS::value_type,
                            typename tag_sum_result<typename LHS::tag_type, typename RHS::tag_type>::type>;
    };

    template <typename LHS, typename RHS>
        requires requires { typename LHS::tag_type; typename RHS::tag_type; }
    struct difference_result
    {
        using type = Strong<typename LHS::value_type,
                            typename tag_difference_result<typename LHS::tag_type, typename RHS::tag_type>::type>;
    };

    template <typename LHS, typename RHS>
        requires requires { typename LHS::tag_type; typename RHS::tag_type; }
    struct product_result
    {
        using type = Strong<typename LHS::value_type,
                            typename tag_product_result<typename LHS::tag_type, typename RHS::tag_type>::type>;
    };

    template <typename LHS, typename RHS>
        requires requires { typename LHS::tag_type; typename RHS::tag_type; }
    struct quotient_result
    {
        using type = Strong<typename LHS::value_type,
                            typename tag_quotient_result<typename LHS::tag_type, typename RHS::tag_type>::type>;
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

    // ---- arithmetic ops (tagged only) ----

    template <typename LHS, typename RHS>
        requires requires { typename sum_result<LHS, RHS>::type; }
    [[nodiscard]] constexpr auto operator+(const LHS &lhs, const RHS &rhs)
        -> typename sum_result<LHS, RHS>::type
    {
        return typename sum_result<LHS, RHS>::type{lhs.get() + rhs.get()};
    }

    template <typename LHS, typename RHS>
        requires requires { typename difference_result<LHS, RHS>::type; }
    [[nodiscard]] constexpr auto operator-(const LHS &lhs, const RHS &rhs)
        -> typename difference_result<LHS, RHS>::type
    {
        return typename difference_result<LHS, RHS>::type{lhs.get() - rhs.get()};
    }

    template <typename LHS, typename RHS>
        requires requires { typename product_result<LHS, RHS>::type; }
    [[nodiscard]] constexpr auto operator*(const LHS &lhs, const RHS &rhs)
        -> typename product_result<LHS, RHS>::type
    {
        return typename product_result<LHS, RHS>::type{lhs.get() * rhs.get()};
    }

    template <typename LHS, typename RHS>
        requires requires { typename quotient_result<LHS, RHS>::type; }
    [[nodiscard]] constexpr auto operator/(const LHS &lhs, const RHS &rhs)
        -> typename quotient_result<LHS, RHS>::type
    {
        return typename quotient_result<LHS, RHS>::type{lhs.get() / rhs.get()};
    }

    // ---- scalar overloads ----

    template <typename T, typename TAG, Scalar S>
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
    [[nodiscard]] constexpr auto operator/(const Strong<T, TAG> &lhs, S scalar)
        -> scalar_div_result_t<Strong<T, TAG>, S>
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
    [[nodiscard]] constexpr auto operator/(S scalar, const Strong<T, TAG> &rhs)
        -> typename quotient_result<Strong<S, void>, Strong<T, TAG>>::type
    {
        return typename quotient_result<Strong<S, void>, Strong<T, TAG>>::type{
            static_cast<T>(scalar) / rhs.get()};
    }

    // ---- compound assignment ----

    template <typename T, typename TAG>
    constexpr Strong<T, TAG> &operator+=(Strong<T, TAG> &lhs, const Strong<T, TAG> &rhs)
    {
        lhs = lhs + rhs;
        return lhs;
    }

    template <typename T, typename TAG>
    constexpr Strong<T, TAG> &operator-=(Strong<T, TAG> &lhs, const Strong<T, TAG> &rhs)
    {
        lhs = lhs - rhs;
        return lhs;
    }

    template <typename T, typename TAG, Scalar S>
    constexpr Strong<T, TAG> &operator*=(Strong<T, TAG> &lhs, S scalar)
    {
        lhs = lhs * scalar;
        return lhs;
    }

    // compound assignment for scalar division, only if scalar_div_result matches Strong<T, TAG>
    template <typename T, typename TAG, Scalar S>
    constexpr auto operator/=(Strong<T, TAG> &lhs, S scalar)
        -> std::enable_if_t<
            std::is_same_v<scalar_div_result_t<Strong<T, TAG>, S>, Strong<T, TAG>>,
            Strong<T, TAG> &>
    {
        lhs = lhs / scalar;
        return lhs;
    }

    // ---- unary ----

    template <typename T, typename TAG>
    [[nodiscard]] constexpr Strong<T, TAG> operator-(const Strong<T, TAG> &v)
    {
        return Strong<T, TAG>(-v.get());
    }

    template <typename T, typename TAG>
    [[nodiscard]] constexpr bool operator==(const Strong<T, TAG> &lhs, const Strong<T, TAG> &rhs)
    {
        return lhs.get() == rhs.get();
    }

} // namespace strong_types
