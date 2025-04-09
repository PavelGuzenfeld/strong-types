#include "strong-types/si.hpp"

using namespace strong_types;

using Length = unit_t<float, LengthTag>;
using Mass = unit_t<float, MassTag>;
using Time = unit_t<float, TimeTag>;
using Speed = unit_t<float, SpeedTag>;
using Acceleration = unit_t<float, AccelerationTag>;
using Force = unit_t<float, ForceTag>;
using Energy = unit_t<float, EnergyTag>;
using Area = unit_t<float, AreaTag>;
using Hertz = unit_t<float, HertzTag>;
using Celsius = unit_t<float, CelsiusTag>;
using Volt = unit_t<float, VoltTag>;
using Radian = unit_t<float, RadianTag>;
using Steradian = unit_t<float, SteradianTag>;

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

static_assert([]
              {
    constexpr Length d{0.0f};
    constexpr auto result = d * 123.456f;
    return result.get() == 0.0f; }(), "zero multiplication failed");

static_assert([]
              {
    constexpr Length d{1e30f};
    constexpr auto result = d / 1e5f;
    constexpr float expected = 1e25f;
    constexpr float epsilon = 1e20f;
    return (result.get() > expected - epsilon) && (result.get() < expected + epsilon); }(), "large value division failed");

static_assert([]
              {
    constexpr Length d{3.0f};
    constexpr auto x = 2 * d;
    constexpr auto y = d * 2U;
    constexpr auto z = d * 2.0;
    return x.get() == 6.0f && y.get() == 6.0f && z.get() == 6.0f; }(), "mixed scalar overloads failed");

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
    constexpr Length l{4.0f};
    constexpr Area a = l * l;
    return a.get() == 16.0f; }(), "area = length * length failed");

static_assert([]
              {
    constexpr Radian rad{3.14f};
    constexpr Radian x = rad + rad;
    return x.get() == 6.28f; }(), "radian addition failed");

static_assert([]
              {
    constexpr Time t{0.5f};
    constexpr Hertz hz = 1.0f / t;
    return hz.get() == 2.0f; }(), "hz = 1 / time failed");
