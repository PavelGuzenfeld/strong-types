// NOLINTBEGIN(readability-magic-numbers,readability-identifier-length) — test file
#include "strong-types/si_literals.hpp"
#include "strong-types/si_scaled.hpp"
#include "strong-types/si_scaled_literals.hpp"

using namespace strong_types;
using namespace strong_types::si_literals;
using namespace strong_types::si_scaled_literals;

// ---- MassTag addition (regression test for bug fix) ----

static_assert(
    [] {
        constexpr auto m1 = 2.0_kg;
        constexpr auto m2 = 3.0_kg;
        constexpr auto sum = m1 + m2;
        return sum.get() == 5.0;
    }(),
    "mass addition: 2kg + 3kg = 5kg");

static_assert(
    [] {
        constexpr auto m1 = 5.0_kg;
        constexpr auto m2 = 1.5_kg;
        constexpr auto diff = m1 - m2;
        return diff.get() == 3.5;
    }(),
    "mass subtraction: 5kg - 1.5kg = 3.5kg");

// ---- Negative values in scaled conversions ----

static_assert((-5.0_km).to_base().get() == -5000.0, "negative km to base");
static_assert((-100.0_ms).to_base().get() == -0.1, "negative ms to base");
static_assert((-2.5_g).to_base().get() == -0.0025, "negative g to base");
static_assert(
    [] {
        constexpr auto result = -3.0_km + 1.0_km;
        return result.get() == -2.0;
    }(),
    "negative scaled addition");

// ---- Very large numbers ----

static_assert(
    [] {
        constexpr auto big = Kilometers<double>{1e12};
        constexpr auto base = big.to_base();
        return base.get() == 1e15;
    }(),
    "very large km to m");

static_assert(
    [] {
        constexpr auto big = unit_t<double, LengthTag>{1e15};
        constexpr auto scaled = from_base<Kilometers<double>>(big);
        return scaled.get() == 1e12;
    }(),
    "very large m to km via from_base");

// ---- Very small numbers ----

static_assert(
    [] {
        constexpr auto tiny = Nanoseconds<double>{0.1};
        constexpr auto base = tiny.to_base();
        return base.get() > 9.9e-11 && base.get() < 1.01e-10;
    }(),
    "very small ns to s");

static_assert(
    [] {
        constexpr auto tiny = Micrometers<double>{0.001};
        constexpr auto base = tiny.to_base();
        return base.get() > 9.9e-10 && base.get() < 1.01e-9;
    }(),
    "very small um to m");

// ---- Integer types with ScaledUnit ----

static_assert(
    [] {
        constexpr ScaledUnit<int, LengthTag, std::kilo> km{5};
        return km.to_base().get() == 5000;
    }(),
    "integer km to base");

static_assert(
    [] {
        constexpr ScaledUnit<int, TimeTag, std::ratio<60>> mins{2};
        return mins.to_base().get() == 120;
    }(),
    "integer minutes to seconds");

static_assert(
    [] {
        constexpr ScaledUnit<int, LengthTag, std::kilo> a{3};
        constexpr ScaledUnit<int, LengthTag, std::kilo> b{7};
        constexpr auto sum = a + b;
        return sum.get() == 10;
    }(),
    "integer scaled addition");

// ---- Default construction of new types ----

static_assert(unit_t<double, PowerTag>{}.get() == 0.0, "default power is 0");
static_assert(unit_t<double, PressureTag>{}.get() == 0.0, "default pressure is 0");
static_assert(unit_t<double, AngularVelocityTag>{}.get() == 0.0, "default angular velocity is 0");
static_assert(unit_t<double, VolumeTag>{}.get() == 0.0, "default volume is 0");
static_assert(unit_t<double, DensityTag>{}.get() == 0.0, "default density is 0");
static_assert(Micrometers<double>{}.get() == 0.0, "default um is 0");
static_assert(Days<double>{}.get() == 0.0, "default day is 0");
static_assert(Weeks<double>{}.get() == 0.0, "default week is 0");
static_assert(Tons<double>{}.get() == 0.0, "default ton is 0");
static_assert(KilometersPerHour<double>{}.get() == 0.0, "default km/h is 0");

// ---- Cross-scale edge cases ----

static_assert(
    [] {
        constexpr auto result = 1.0_km + 500000.0_mm;
        return result.get() == 1500.0;
    }(),
    "km + mm = base meters");

static_assert(
    [] {
        constexpr auto result = 1.0_hr + 1000000000.0_ns;
        constexpr double expected = 3601.0;
        return result.get() > expected - 0.001 && result.get() < expected + 0.001;
    }(),
    "hr + ns = base seconds");

static_assert(
    [] {
        constexpr auto result = 1.0_km + 100.0_um;
        return result.get() > 1000.0 && result.get() < 1000.001;
    }(),
    "km + um = base meters");

// ---- New unit dimensional algebra: Power ----

static_assert(
    [] {
        constexpr auto energy = 100.0_J;
        constexpr auto time = 5.0_s;
        constexpr auto power = energy / time;
        return power.get() == 20.0;
    }(),
    "power = energy / time");

static_assert(
    [] {
        constexpr auto power = 20.0_W;
        constexpr auto time = 5.0_s;
        constexpr auto energy = power * time;
        return energy.get() == 100.0;
    }(),
    "energy = power * time");

// ---- New unit dimensional algebra: Pressure ----

static_assert(
    [] {
        constexpr auto force = 1000.0_N;
        constexpr auto area = 10.0_m2;
        constexpr auto pressure = force / area;
        return pressure.get() == 100.0;
    }(),
    "pressure = force / area");

static_assert(
    [] {
        constexpr auto pressure = 100.0_Pa;
        constexpr auto area = 10.0_m2;
        constexpr auto force = pressure * area;
        return force.get() == 1000.0;
    }(),
    "force = pressure * area");

// ---- New unit dimensional algebra: AngularVelocity ----

static_assert(
    [] {
        constexpr auto angle = 6.28_rad;
        constexpr auto time = 2.0_s;
        constexpr auto angular_vel = angle / time;
        return angular_vel.get() == 3.14;
    }(),
    "angular velocity = radian / time");

static_assert(
    [] {
        constexpr auto angular_vel = 3.14_rps;
        constexpr auto time = 2.0_s;
        constexpr auto angle = angular_vel * time;
        return angle.get() == 6.28;
    }(),
    "radian = angular velocity * time");

// ---- New unit dimensional algebra: Volume ----

static_assert(
    [] {
        constexpr auto length = 5.0_m;
        constexpr auto area = 10.0_m2;
        constexpr auto volume = length * area;
        return volume.get() == 50.0;
    }(),
    "volume = length * area");

static_assert(
    [] {
        constexpr auto volume = 50.0_m3;
        constexpr auto length = 5.0_m;
        constexpr auto area = volume / length;
        return area.get() == 10.0;
    }(),
    "area = volume / length");

static_assert(
    [] {
        constexpr auto volume = 50.0_m3;
        constexpr auto area = 10.0_m2;
        constexpr auto length = volume / area;
        return length.get() == 5.0;
    }(),
    "length = volume / area");

// ---- New unit dimensional algebra: Density ----

static_assert(
    [] {
        constexpr auto mass = 100.0_kg;
        constexpr auto volume = 2.0_m3;
        constexpr auto density = mass / volume;
        return density.get() == 50.0;
    }(),
    "density = mass / volume");

static_assert(
    [] {
        constexpr unit_t<double, DensityTag> density{50.0};
        constexpr auto volume = 2.0_m3;
        constexpr auto mass = density * volume;
        return mass.get() == 100.0;
    }(),
    "mass = density * volume");

// ---- Same-tag division -> scalar for new types ----

static_assert(
    [] {
        constexpr auto p1 = 100.0_W;
        constexpr auto p2 = 25.0_W;
        constexpr double ratio = p1 / p2;
        return ratio == 4.0;
    }(),
    "power / power = scalar");

static_assert(
    [] {
        constexpr auto f1 = 100.0_N;
        constexpr auto f2 = 50.0_N;
        constexpr double ratio = f1 / f2;
        return ratio == 2.0;
    }(),
    "force / force = scalar");

static_assert(
    [] {
        constexpr auto m1 = 6.0_kg;
        constexpr auto m2 = 2.0_kg;
        constexpr double ratio = m1 / m2;
        return ratio == 3.0;
    }(),
    "mass / mass = scalar");

// ---- from_base() round-trips ----

static_assert(
    [] {
        constexpr auto base = unit_t<double, LengthTag>{1000.0};
        constexpr auto km = from_base<Kilometers<double>>(base);
        return km.get() == 1.0;
    }(),
    "from_base: 1000m -> 1km");

static_assert(
    [] {
        constexpr auto base = unit_t<double, TimeTag>{3600.0};
        constexpr auto hr = from_base<Hours<double>>(base);
        return hr.get() == 1.0;
    }(),
    "from_base: 3600s -> 1hr");

static_assert(
    [] {
        constexpr auto base = unit_t<double, TimeTag>{86400.0};
        constexpr auto day = from_base<Days<double>>(base);
        return day.get() == 1.0;
    }(),
    "from_base: 86400s -> 1d");

static_assert(
    [] {
        constexpr auto base = unit_t<double, MassTag>{1000.0};
        constexpr auto ton = from_base<Tons<double>>(base);
        return ton.get() == 1.0;
    }(),
    "from_base: 1000kg -> 1t");

static_assert(
    [] {
        constexpr auto orig = 5.0_km;
        constexpr auto round_trip = from_base<Kilometers<double>>(orig.to_base());
        return round_trip.get() == 5.0;
    }(),
    "from_base round-trip: km -> m -> km");

static_assert(
    [] {
        constexpr auto orig = 250.0_ms;
        constexpr auto round_trip = from_base<Milliseconds<double>>(orig.to_base());
        return round_trip.get() == 250.0;
    }(),
    "from_base round-trip: ms -> s -> ms");

// ---- checked_cast ----

static_assert(
    [] {
        constexpr auto km = Kilometers<double>{5.0};
        constexpr auto mm = checked_cast<Millimeters<double>>(km);
        return mm.get() == 5000000.0;
    }(),
    "checked_cast: 5km -> 5000000mm");

static_assert(
    [] {
        constexpr auto hr = Hours<double>{2.0};
        constexpr auto mins = checked_cast<Minutes<double>>(hr);
        return mins.get() == 120.0;
    }(),
    "checked_cast: 2hr -> 120min");

// ---- KilometersPerHour conversions ----

static_assert(
    [] {
        constexpr auto speed = KilometersPerHour<double>{36.0};
        constexpr auto base = speed.to_base();
        return base.get() == 10.0;
    }(),
    "36 km/h = 10 m/s");

static_assert(
    [] {
        constexpr auto speed = KilometersPerHour<double>{72.0};
        constexpr auto base = speed.to_base();
        return base.get() == 20.0;
    }(),
    "72 km/h = 20 m/s");

static_assert(
    [] {
        constexpr auto base = unit_t<double, SpeedTag>{10.0};
        constexpr auto kmh = from_base<KilometersPerHour<double>>(base);
        return kmh.get() == 36.0;
    }(),
    "from_base: 10 m/s -> 36 km/h");

static_assert(
    [] {
        constexpr auto speed = 36.0_kmh;
        constexpr auto base = speed.to_base();
        return base.get() == 10.0;
    }(),
    "UDL: 36_kmh = 10 m/s");

// ---- New scaled UDLs ----

static_assert((1.0_um).to_base().get() == 0.000001, "1um = 1e-6 m");
static_assert((1.0_d).to_base().get() == 86400.0, "1d = 86400s");
static_assert((1.0_wk).to_base().get() == 604800.0, "1wk = 604800s");
static_assert((1.0_t).to_base().get() == 1000.0, "1t = 1000kg");

// ---- Mass scaled operations ----

static_assert(
    [] {
        constexpr auto result = 1.0_t + 500.0_g;
        return result.get() > 1000.4999 && result.get() < 1000.5001;
    }(),
    "1t + 500g = 1000.5kg");

static_assert(
    [] {
        constexpr auto result = 2000.0_g + 3.0_kg;
        return result.get() == 5.0;
    }(),
    "2000g + 3kg = 5kg");

// ---- Days/Weeks operations ----

static_assert(
    [] {
        constexpr auto result = 1.0_wk + 1.0_d;
        return result.get() == 691200.0;
    }(),
    "1wk + 1d = 8 days in seconds");

static_assert(
    [] {
        constexpr auto day = Days<double>{1.0};
        constexpr auto hr = checked_cast<Hours<double>>(day);
        return hr.get() == 24.0;
    }(),
    "1 day = 24 hours");

// NOLINTEND(readability-magic-numbers,readability-identifier-length)
