#include "strong-types/strong.hpp"

namespace strong_types
{
    struct DistanceTag
    {
    };
    using Distance = Strong<float, DistanceTag>;

    template <>
    struct sum_result<Distance, Distance>
    {
        using type = Distance;
    };

    template <>
    struct difference_result<Distance, Distance>
    {
        using type = Distance;
    };
}

using strong_types::Distance;

static_assert([]
              {
    constexpr Distance a{10.0f};
    constexpr Distance b{5.0f};
    constexpr auto sum = a + b;
    constexpr auto diff = a - b;
    return sum.get() == 15.0f && diff.get() == 5.0f; }(), "compile-time addition/subtraction failed");

static_assert([]
              {
    constexpr Distance d{10.0f};
    constexpr auto scaled = d * 2.0f;
    constexpr auto half = d / 2.0f;
    return scaled.get() == 20.0f && half.get() == 5.0f; }(), "compile-time mul/div failed");

static_assert([]
              {
    constexpr Distance a{42.0f};
    constexpr Distance b{42.0f};
    return (a <=> b) == std::strong_ordering::equal; }(), "spaceship comparison test failed");

static_assert([]
              {
    constexpr Distance d{};
    return d.get() == 0.0f; }(), "default constructor failed");

static_assert([]
              {
    constexpr Distance a{3.0f};
    constexpr Distance neg = -a;
    return neg.get() == -3.0f; }(), "negation test failed");

static_assert([]
              {
    Distance d{3.0f};
    d += Distance{2.0f};
    d *= 3.0f;
    d -= Distance{3.0f};
    d /= 2.0f;
    return d.get() == 6.0f; }(), "compound assignment test failed");

static_assert([]
              {
    constexpr Distance a{5.0f};
    constexpr Distance b{5.0f};
    constexpr Distance c{7.0f};
    return a == b && !(a == c); }(), "explicit equality test failed");

static_assert([]
              {
    constexpr Distance d{4.0f};
    constexpr auto a = d * 2;   // int
    constexpr auto b = d / 2u;  // unsigned
    constexpr auto c = 2.5 * d; // double
    return a.get() == 8.0f && b.get() == 2.0f && c.get() == 10.0f; }(), "generic scalar math failed");

int main()
{
    return 0;
}
