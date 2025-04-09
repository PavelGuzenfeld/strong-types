#pragma once

#include <algorithm>
#include <array> // for sanity
#include <compare>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

template <typename T, std::size_t N>
class alignas(64) AlignedArray
{
public:
    static_assert(std::is_arithmetic_v<T>, "only numeric types allowed — keep your strings in std::vector");

    using value_type = T;
    using iterator = T *;
    using const_iterator = const T *;
    using size_type = std::size_t;

    // default constructor
    constexpr AlignedArray() noexcept = default;

    template <typename... Args>
        requires(sizeof...(Args) == N) && (std::is_convertible_v<Args, T> && ...)
    constexpr AlignedArray(Args... args) noexcept
        : data_{static_cast<T>(args)...}
    {
    }

    // stl-style accessors
    [[nodiscard]] constexpr iterator begin() noexcept { return data_; }
    [[nodiscard]] constexpr const_iterator begin() const noexcept { return data_; }
    [[nodiscard]] constexpr iterator end() noexcept { return data_ + N; }
    [[nodiscard]] constexpr const_iterator end() const noexcept { return data_ + N; }

    [[nodiscard]] constexpr size_type size() const noexcept { return N; }

    [[nodiscard]] constexpr T &operator[](std::size_t i) noexcept { return data_[i]; }
    [[nodiscard]] constexpr const T &operator[](std::size_t i) const noexcept { return data_[i]; }

    // vector ops
    constexpr AlignedArray &operator+=(const AlignedArray &rhs) noexcept
    {
        std::transform(begin(), end(), rhs.begin(), begin(), std::plus<>{});
        return *this;
    }

    constexpr AlignedArray &operator-=(const AlignedArray &rhs) noexcept
    {
        std::transform(begin(), end(), rhs.begin(), begin(), std::minus<>{});
        return *this;
    }

    constexpr AlignedArray &operator*=(const AlignedArray &rhs) noexcept
    {
        std::transform(begin(), end(), rhs.begin(), begin(), std::multiplies<>{});
        return *this;
    }

    constexpr AlignedArray &operator/=(const AlignedArray &rhs) noexcept
    {
        std::transform(begin(), end(), rhs.begin(), begin(), std::divides<>{});
        return *this;
    }

    // scalar ops
    constexpr AlignedArray &operator*=(T scalar) noexcept
    {
        std::transform(begin(), end(), begin(), [scalar](T v)
                       { return v * scalar; });
        return *this;
    }

    constexpr AlignedArray &operator/=(T scalar) noexcept
    {
        std::transform(begin(), end(), begin(), [scalar](T v)
                       { return v / scalar; });
        return *this;
    }

    // unary minus
    [[nodiscard]] constexpr AlignedArray operator-() const noexcept
    {
        AlignedArray out{};
        std::transform(begin(), end(), out.begin(), std::negate<>{});
        return out;
    }

    [[nodiscard]] constexpr bool operator==(const AlignedArray &rhs) const noexcept
    {
        return std::equal(begin(), end(), rhs.begin());
    }

    [[nodiscard]] constexpr std::partial_ordering operator<=>(const AlignedArray &rhs) const noexcept
    {
        for (std::size_t i = 0; i < N; ++i)
        {
            if (auto cmp = data_[i] <=> rhs.data_[i]; cmp != 0)
                return cmp;
        }
        return std::strong_ordering::equal;
    }

private:
    T data_[N]{};
};

// binary ops
template <typename T, std::size_t N>
[[nodiscard]] constexpr AlignedArray<T, N> operator+(AlignedArray<T, N> lhs, const AlignedArray<T, N> &rhs)
{
    return lhs += rhs;
}

template <typename T, std::size_t N>
[[nodiscard]] constexpr AlignedArray<T, N> operator-(AlignedArray<T, N> lhs, const AlignedArray<T, N> &rhs)
{
    return lhs -= rhs;
}

template <typename T, std::size_t N>
[[nodiscard]] constexpr AlignedArray<T, N> operator*(AlignedArray<T, N> lhs, const AlignedArray<T, N> &rhs)
{
    return lhs *= rhs;
}

template <typename T, std::size_t N>
[[nodiscard]] constexpr AlignedArray<T, N> operator/(AlignedArray<T, N> lhs, const AlignedArray<T, N> &rhs)
{
    return lhs /= rhs;
}

template <typename T, std::size_t N>
[[nodiscard]] constexpr AlignedArray<T, N> operator*(AlignedArray<T, N> lhs, T scalar)
{
    return lhs *= scalar;
}

template <typename T, std::size_t N>
[[nodiscard]] constexpr AlignedArray<T, N> operator/(AlignedArray<T, N> lhs, T scalar)
{
    return lhs /= scalar;
}
