#include "strong-types/si.hpp"
#include <cstdint> // for fixed width integer types

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

using LengthI32 = unit_t<std::int32_t, LengthTag>;
using TimeU64 = unit_t<std::uint64_t, TimeTag>;
using SpeedD = unit_t<double, SpeedTag>;
using EnergyD = unit_t<double, EnergyTag>;

using LengthI32 = unit_t<std::int32_t, LengthTag>;
using TimeU64 = unit_t<std::uint64_t, TimeTag>;
using SpeedD = unit_t<double, SpeedTag>;
using EnergyD = unit_t<double, EnergyTag>;

static_assert([]
            {
    constexpr LengthI32 d{static_cast<std::int32_t>(300)};
    constexpr TimeU64 t{static_cast<std::uint64_t>(20)};
    constexpr SpeedD s{static_cast<double>(d.get()) / static_cast<double>(t.get())};
    return s.get() == 15.0; }(), "✅ correct casting: int32_t / uint64_t -> double = SpeedD");

static_assert([]
              {
    constexpr unit_t<std::uint16_t, TimeTag> t{static_cast<std::uint16_t>(4)};
    constexpr auto raw = static_cast<int>(8) / static_cast<int>(t.get());
    constexpr unit_t<std::uint16_t, HertzTag> hz{static_cast<std::uint16_t>(raw)};
    return hz.get() == 2; }(), "✅ correct casting: scalar / strong<uint16_t> = strong<uint16_t>");

static_assert([]
              {
    constexpr unit_t<std::int16_t, LengthTag> l{static_cast<std::int16_t>(5)};
    constexpr auto raw = static_cast<int>(l.get()) * static_cast<int>(l.get());
    constexpr unit_t<std::int16_t, AreaTag> a{static_cast<std::int16_t>(raw)};
    return a.get() == 25; }(), "✅ correct casting: int16_t * int16_t = Area");

static_assert([]
              {
    constexpr unit_t<double, ForceTag> f{3.5};
    constexpr unit_t<double, LengthTag> d{2.0};
    constexpr EnergyD e{f.get() * d.get()};
    return e.get() == 7.0; }(), "✅ correct casting: double force * length = energy");

static_assert([]
              {
    constexpr auto t = unit_t<std::int32_t, TimeTag>{static_cast<std::int32_t>(4)};
    constexpr auto raw = static_cast<double>(8.0) / static_cast<double>(t.get());
    constexpr unit_t<double, HertzTag> hz{raw};
    return hz.get() == 2.0; }(), "✅ correct casting: scalar double / int32_t strong = double hertz");
