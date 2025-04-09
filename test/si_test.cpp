#include "strong-types/si.hpp"

using namespace strong_types;

using Length = unit_t<LengthTag>;
using Mass = unit_t<MassTag>;
using Time = unit_t<TimeTag>;
using Speed = unit_t<SpeedTag>;
using Acceleration = unit_t<AccelerationTag>;
using Force = unit_t<ForceTag>;
using Energy = unit_t<EnergyTag>;

static_assert([]
              {
    constexpr Length d{100.0f};
    constexpr Time t{20.0f};
    constexpr Speed s = d / t;
    return s.get() == 5.0f; }(), "Length / Time = Speed failed");

static_assert([]
              {
    constexpr Speed s{5.0f};
    constexpr Time t{10.0f};
    constexpr Length d = s * t;
    return d.get() == 50.0f; }(), "Speed * Time = Length failed");

static_assert([]
              {
    constexpr Length d1{50.0f};
    constexpr Length d2{100.0f};
    constexpr float ratio = d1 / d2;
    return ratio == 0.5f; }(), "Length / Length = scalar ratio failed");

// edge case: zero
static_assert([]
              {
    constexpr Length d{0.0f};
    constexpr auto result = d * 123.456f;
    return result.get() == 0.0f; }(), "zero multiplication failed");

// edge case: large values
static_assert([]
              {
    constexpr Length d{1e30f};
    constexpr auto result = d / 1e5f;
    constexpr float expected = 1e25f;
    constexpr float epsilon = 1e20f;
    return (result.get() > expected - epsilon) && (result.get() < expected + epsilon); }(), "large value division failed");

// edge case: mixed scalar multiplication
static_assert([]
              {
    constexpr Length d{3.0f};
    constexpr auto x = 2 * d;
    constexpr auto y = d * 2U;
    constexpr auto z = d * 2.0;
    return x.get() == 6.0f && y.get() == 6.0f && z.get() == 6.0f; }(), "mixed scalar overloads failed");

// BAD case: mismatched tag addition (should not compile, uncomment to verify)
/*
constexpr Time t1{5.0f};
constexpr Length d1{10.0f};
auto bad = t1 + d1; // error: no sum_result<Time, Length>
*/

// BAD case: divide by zero — won't fail at compile time but... kaboom at runtime if you try it

static_assert([]
              {
    constexpr Length d{100.0f};
    constexpr Time t{10.0f};
    constexpr Speed v = d / t;
    return v.get() == 10.0f; }(), "speed = length / time failed");

static_assert([]
              {
    constexpr Speed v{10.0f};
    constexpr Time t{2.0f};
    constexpr Acceleration a = v / t;
    return a.get() == 5.0f; }(), "acceleration = speed / time failed");

static_assert([]
              {
    constexpr Mass m{2.0f};
    constexpr Acceleration a{10.0f};
    constexpr Force f = m * a;
    return f.get() == 20.0f; }(), "force = mass * acceleration failed");

static_assert([]
              {
    constexpr Force f{10.0f};
    constexpr Length d{3.0f};
    constexpr Energy e = f * d;
    return e.get() == 30.0f; }(), "energy = force * Length failed");

static_assert([]
              {
    constexpr auto d = 3.0_m;
    constexpr auto t = 1.5_s;
    constexpr auto v = d / t;
    return v.get() == 2.0f; }(), "literals + speed failed");
