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

    struct DummyTag
    {
    };
    struct DummyTag2
    {
    };
    using Dummy = Strong<float, DummyTag>;
    using Dummy2 = Strong<float, DummyTag2>;

    template <>
    struct tag_sum_result<DummyTag, DummyTag>
    {
        using type = DummyTag;
    };
    template <>
    struct tag_difference_result<DummyTag, DummyTag>
    {
        using type = DummyTag;
    };
    template <>
    struct tag_product_result<DummyTag, DummyTag>
    {
        using type = DummyTag;
    };
    template <>
    struct tag_product_result<DummyTag2, DummyTag>
    {
        using type = DummyTag;
    };
    template <>
    struct tag_quotient_result<DummyTag, DummyTag>
    {
        using type = void;
    };
    template <>
    struct tag_quotient_result<void, DummyTag>
    {
        using type = DummyTag2;
    };
}

// alias
using strong_types::Dummy;
using strong_types::Dummy2;

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

static_assert([]
              {
    constexpr Dummy d{3.5f};
    constexpr Dummy copy = d;
    return copy.get() == 3.5f; }(), "copy constructor / get() failed");

static_assert([]
              {
    Dummy d1{2.0f};
    Dummy d2{2.0f};
    auto r1 = d1 + d2;
    auto r2 = d1 - d2;
    r1 *= 2;
    r2 += d2;
    return r1.get() == 8.0f && r2.get() == 2.0f; }(), "compound op mix failed");

static_assert([]
              {
    constexpr Dummy d{9.0f};
    constexpr float s = d / d;
    return s == 1.0f; }(), "scalar result division failed");

static_assert([]
              {
    constexpr Dummy d{2.0f};
    constexpr Dummy2 result = 1.0f / d;
    return result.get() == 0.5f; }(), "scalar / strong reversed division failed");

static_assert([]
              {
    constexpr Dummy d1{3.0f};
    constexpr Dummy d2{3.0f};
    constexpr auto result = d1 * d2;
    return result.get() == 9.0f; }(), "same-type product trait failed");

// static_assert([] {
//     Dummy d1{1.0f};
//     Dummy2 d2{2.0f};
//     auto x = d1 + d2; // this should fail to compile — different tags
//     (void)x;
//     return true;
// }(), "invalid tag addition should not compile");

static_assert([]
              {
    Dummy d{4.0f};
    d += Dummy{1.0f};
    d -= Dummy{1.0f};
    d *= 2.0f;
    d /= 4;
    return d.get() == 2.0f; }(), "compound +-=/* ops on mixed scalars");

static_assert([]
              {
    Dummy d1{1.0f};
    Dummy d2{2.0f};
    return (d1 < d2) && (d1 != d2) && !(d1 > d2); }(), "ordering and comparison check");

int main()
{
    return 0;
}
