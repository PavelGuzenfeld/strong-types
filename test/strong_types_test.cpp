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



constexpr bool almost_equal(float a, float b, float epsilon = 0.001f)
{
    return (a - b < epsilon) && (b - a < epsilon);
}
struct Vec2
{
    float x, y;

    constexpr Vec2 operator+(const Vec2 &o) const { return {x + o.x, y + o.y}; }
    constexpr Vec2 operator-(const Vec2 &o) const { return {x - o.x, y - o.y}; }
    constexpr Vec2 operator*(const Vec2 &o) const { return {x * o.x, y * o.y}; }
    constexpr Vec2 operator/(const Vec2 &o) const
    {
        return {
            o.x != 0.0f ? x / o.x : 0.0f,
            o.y != 0.0f ? y / o.y : 0.0f};
    }

    constexpr float ratio() const
    {
        if (x == 0.0f)
            return 0.0f;
        return y / x;
    }

    constexpr Vec2 operator/(float scalar) const { return {x / scalar, y / scalar}; }
    constexpr Vec2 operator*(float scalar) const { return {x * scalar, y * scalar}; }

    constexpr Vec2 operator-() const { return {-x, -y}; }
    constexpr bool operator==(const Vec2 &o) const
    {
        return almost_equal(x, o.x) && almost_equal(y, o.y);
    }
    constexpr std::partial_ordering operator<=>(const Vec2 &o) const
    {
        if (auto cmp = x <=> o.x; cmp != 0)
            return cmp;
        return y <=> o.y;
    }
};



constexpr bool vec2_equal(const Vec2 &a, const Vec2 &b, float epsilon = 0.001f)
{
    return almost_equal(a.x, b.x, epsilon) && almost_equal(a.y, b.y, epsilon);
}

struct DisplacementTag
{
};
using Displacement = strong_types::Strong<Vec2, DisplacementTag>;

template <>
struct strong_types::tag_sum_result<DisplacementTag, DisplacementTag>
{
    using type = DisplacementTag;
};

template <>
struct strong_types::tag_difference_result<DisplacementTag, DisplacementTag>
{
    using type = DisplacementTag;
};

template <>
struct strong_types::tag_product_result<DisplacementTag, void>
{
    using type = DisplacementTag; // scaling
};

template <>
struct strong_types::tag_product_result<void, DisplacementTag>
{
    using type = DisplacementTag;
};

template <>
struct strong_types::tag_quotient_result<DisplacementTag, void>
{
    using type = DisplacementTag; // scalar div
};

template <>
struct strong_types::scalar_division_result<Displacement, float>
{
    using type = Displacement;
};

static_assert([]
              {
    constexpr Displacement d1{Vec2{3.0f, 4.0f}};
    constexpr Displacement d2{Vec2{1.0f, 2.0f}};

    constexpr auto sum = d1 + d2;
    constexpr auto diff = d1 - d2;
    constexpr auto scaled = d1 * 2.0f;
    constexpr auto halved = d1 / 2.0f;

    return sum.get() == Vec2{4.0f, 6.0f} &&
           diff.get() == Vec2{2.0f, 2.0f} &&
           scaled.get() == Vec2{6.0f, 8.0f} &&
           halved.get() == Vec2{1.5f, 2.0f};


            }(), "displacement math test failed");

static_assert([]
              {
    constexpr Displacement big{Vec2{1e30f, 1e-30f}};
    constexpr auto div = big / 10.0f;
    return vec2_equal(div.get(), Vec2{1e29f, 1e-31f}); }(), "high-magnitude division test failed");

static_assert([]
              {
                  constexpr Displacement a{Vec2{1.000001f, 2.000001f}};
                  constexpr Displacement b{Vec2{1.000002f, 2.000002f}};
                  return (a == b); // fail-safe: `Vec2::operator==` uses epsilon
              }(),
              "approximate equality violated");

constexpr Displacement d{Vec2{1e30f, 1e-30f}};
constexpr auto result = d / 1e10f;
static_assert(vec2_equal(result.get(), Vec2{1e20f, 1e-40f}), "oopsie underflow?");

int main()
{
    return 0;
}
