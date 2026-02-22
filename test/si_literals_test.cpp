// NOLINTBEGIN(readability-magic-numbers,readability-identifier-length) — test file
#include "strong-types/si_literals.hpp"

using namespace strong_types;
using namespace strong_types::si_literals;

// Basic construction — floating-point literal
static_assert((100.0_m).get() == 100.0, "floating-point _m literal");

// Basic construction — integer literal
static_assert((20_s).get() == 20.0, "integer _s literal");

// Every suffix compiles and returns the right tag
static_assert(std::is_same_v<decltype(1.0_m)::tag_type, LengthTag>);
static_assert(std::is_same_v<decltype(1.0_kg)::tag_type, MassTag>);
static_assert(std::is_same_v<decltype(1.0_s)::tag_type, TimeTag>);
static_assert(std::is_same_v<decltype(1.0_m2)::tag_type, AreaTag>);
static_assert(std::is_same_v<decltype(1.0_mps)::tag_type, SpeedTag>);
static_assert(std::is_same_v<decltype(1.0_mps2)::tag_type, AccelerationTag>);
static_assert(std::is_same_v<decltype(1.0_N)::tag_type, ForceTag>);
static_assert(std::is_same_v<decltype(1.0_J)::tag_type, EnergyTag>);
static_assert(std::is_same_v<decltype(1.0_Hz)::tag_type, HertzTag>);
static_assert(std::is_same_v<decltype(1.0_degC)::tag_type, CelsiusTag>);
static_assert(std::is_same_v<decltype(1.0_V)::tag_type, VoltTag>);
static_assert(std::is_same_v<decltype(1.0_rad)::tag_type, RadianTag>);
static_assert(std::is_same_v<decltype(1.0_sr)::tag_type, SteradianTag>);

// All literals produce unit_t<double, _>
static_assert(std::is_same_v<decltype(1.0_m)::value_type, double>);
static_assert(std::is_same_v<decltype(42_kg)::value_type, double>);

// Dimensional algebra: Length / Time = Speed
static_assert(
    [] {
        constexpr auto spd = 100.0_m / 20.0_s;
        return spd.get() == 5.0;
    }(),
    "100m / 20s = 5 m/s");

// Dimensional algebra: Mass * Acceleration = Force
static_assert(
    [] {
        constexpr auto f = 2.0_kg * 9.0_mps2;
        return f.get() == 18.0;
    }(),
    "2kg * 9mps2 = 18N");

// Dimensional algebra: Length * Length = Area
static_assert(
    [] {
        constexpr auto a = 10.0_m * 10.0_m;
        return a.get() == 100.0;
    }(),
    "10m * 10m = 100m2");

// Speed * Time = Length
static_assert(
    [] {
        constexpr auto d = 5.0_mps * 10.0_s;
        return d.get() == 50.0;
    }(),
    "5mps * 10s = 50m");

// Force * Length = Energy
static_assert(
    [] {
        constexpr auto e = 10.0_N * 3.0_m;
        return e.get() == 30.0;
    }(),
    "10N * 3m = 30J");

// Addition of same-unit literals
static_assert(
    [] {
        constexpr auto sum = 1.5_rad + 1.5_rad;
        return sum.get() == 3.0;
    }(),
    "1.5rad + 1.5rad = 3rad");

// 1/Time = Hertz
static_assert(
    [] {
        constexpr auto hz = 1.0 / 0.5_s;
        return hz.get() == 2.0;
    }(),
    "1/0.5s = 2Hz");
// NOLINTEND(readability-magic-numbers,readability-identifier-length)
