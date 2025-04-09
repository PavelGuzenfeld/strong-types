#include "strong-types/aligned_array.hpp"
#include "strong-types/strong.hpp"

using namespace strong_types;

struct LengthTag
{
};
struct DistanceTag
{
};
struct VelocityTag
{
};
struct TimeTag
{
};

using Vec = AlignedArray<float, 8>;
using LengthVec = Strong<Vec, LengthTag>;
using DistanceVec = Strong<Vec, DistanceTag>;
using VelocityVec = Strong<Vec, VelocityTag>;
using TimeVec = Strong<Vec, TimeTag>;

namespace strong_types
{

    // length math ops
    template <>
    struct tag_sum_result<LengthTag, LengthTag>
    {
        using type = LengthTag;
    };

    template <>
    struct tag_difference_result<LengthTag, LengthTag>
    {
        using type = LengthTag;
    };

    template <>
    struct tag_product_result<LengthTag, void>
    {
        using type = LengthTag;
    };

    template <>
    struct tag_product_result<void, LengthTag>
    {
        using type = LengthTag;
    };

    template <>
    struct tag_quotient_result<LengthTag, void>
    {
        using type = LengthTag;
    };

    template <>
    struct scalar_division_result<LengthVec, float>
    {
        using type = LengthVec;
    };

    // kinematics trait plumbing
    template <>
    struct tag_quotient_result<DistanceTag, TimeTag>
    {
        using type = VelocityTag;
    };

    template <>
    struct tag_product_result<VelocityTag, TimeTag>
    {
        using type = DistanceTag;
    };

    template <>
    struct tag_quotient_result<DistanceTag, VelocityTag>
    {
        using type = TimeTag;
    };

    template <>
    struct scalar_division_result<DistanceVec, float>
    {
        using type = DistanceVec;
    };

    template <>
    struct scalar_division_result<VelocityVec, float>
    {
        using type = VelocityVec;
    };

    template <>
    struct scalar_division_result<TimeVec, float>
    {
        using type = TimeVec;
    };
} // namespace strong_types

// helper to generate dummy vectors
constexpr Vec make_seq(float start, float step = 1.0f)
{
    Vec out{};
    for (std::size_t i = 0; i < 8; ++i)
    {
        out[i] = start + i * step;
    }
    return out;
}

// test vector arithmetic on LengthVec
static_assert([]
              {
    constexpr LengthVec a{make_seq(1.0f)};
    constexpr LengthVec b{make_seq(2.0f)};
    constexpr auto sum = a + b;
    constexpr auto diff = b - a;
    constexpr auto scaled = a * 2.0f;
    constexpr auto halved = b / 2.0f;

    for (std::size_t i = 0; i < 8; ++i) {
        if (sum.get()[i] != a.get()[i] + b.get()[i]) return false;
        if (diff.get()[i] != b.get()[i] - a.get()[i]) return false;
        if (scaled.get()[i] != a.get()[i] * 2.0f) return false;
        if (halved.get()[i] != b.get()[i] / 2.0f) return false;
    }
    return true; }(), "strong length vector ops failed");

// composite kinematic container
struct Kinematics
{
    DistanceVec distance;
    VelocityVec velocity;
    TimeVec time;

    constexpr void compute_velocity()
    {
        velocity = distance / time;
    }

    constexpr void compute_distance()
    {
        distance = velocity * time;
    }

    constexpr void compute_time()
    {
        time = distance / velocity;
    }
};
static_assert([]
              {
    constexpr DistanceVec d{make_seq(10.0f)};
    constexpr TimeVec t{make_seq(1.0f)};
    constexpr auto v = d / t;

    constexpr Vec expected = [] {
        Vec tmp{};
        tmp[0] = 10.0f / 1.0f;
        tmp[1] = 11.0f / 2.0f;
        tmp[2] = 12.0f / 3.0f;
        tmp[3] = 13.0f / 4.0f;
        tmp[4] = 14.0f / 5.0f;
        tmp[5] = 15.0f / 6.0f;
        tmp[6] = 16.0f / 7.0f;
        tmp[7] = 17.0f / 8.0f;
        return tmp;
    }();

    constexpr VelocityVec expected_v{expected};

    return v.get()[0] == expected_v.get()[0] &&
           v.get()[1] == expected_v.get()[1] &&
           v.get()[2] == expected_v.get()[2] &&
           v.get()[3] == expected_v.get()[3] &&
           v.get()[4] == expected_v.get()[4] &&
           v.get()[5] == expected_v.get()[5] &&
           v.get()[6] == expected_v.get()[6] &&
           v.get()[7] == expected_v.get()[7]; }(), "kinematics vector math failed");

static_assert([]
              {
    constexpr Vec a{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    constexpr Vec b{1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    constexpr Vec expected{2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};

    constexpr auto sum = a + b;
    return sum == expected; }(), "aligned array brace init or transform failed");
