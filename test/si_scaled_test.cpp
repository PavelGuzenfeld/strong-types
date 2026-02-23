// NOLINTBEGIN(readability-magic-numbers,readability-identifier-length) — test file
#include "strong-types/si_literals.hpp"
#include "strong-types/si_scaled.hpp"
#include "strong-types/si_scaled_literals.hpp"

using namespace strong_types;
using namespace strong_types::si_scaled_literals;
using namespace strong_types::si_literals;

// ---- construction ----

static_assert((5.0_km).get() == 5.0, "km construction");
static_assert((100.0_ms).get() == 100.0, "ms construction");
static_assert((250.0_g).get() == 250.0, "g construction");
static_assert((10_mm).get() == 10.0, "mm integer construction");

// ---- tag types ----

static_assert(std::is_same_v<decltype(1.0_km)::tag_type, LengthTag>);
static_assert(std::is_same_v<decltype(1.0_ms)::tag_type, TimeTag>);
static_assert(std::is_same_v<decltype(1.0_g)::tag_type, MassTag>);

// ---- to_base() ----

static_assert((1.0_km).to_base().get() == 1000.0, "1km = 1000m");
static_assert((2.5_km).to_base().get() == 2500.0, "2.5km = 2500m");
static_assert((100.0_cm).to_base().get() == 1.0, "100cm = 1m");
static_assert((1000.0_mm).to_base().get() == 1.0, "1000mm = 1m");
static_assert((1.0_hr).to_base().get() == 3600.0, "1hr = 3600s");
static_assert((1.0_min).to_base().get() == 60.0, "1min = 60s");
static_assert((1000.0_ms).to_base().get() == 1.0, "1000ms = 1s");
static_assert((1000.0_g).to_base().get() == 1.0, "1000g = 1kg");

// ---- in<>() scale conversion ----

static_assert((1.0_km).in<std::ratio<1>>().get() == 1000.0, "1km in meters");
static_assert((1.0_km).in<std::milli>().get() == 1000000.0, "1km in mm");
static_assert((1.0_hr).in<std::ratio<60>>().get() == 60.0, "1hr in minutes");

// ---- same-scale add/sub ----

static_assert(
    [] {
        constexpr auto result = 1.0_km + 2.0_km;
        return result.get() == 3.0;
    }(),
    "1km + 2km = 3km");

static_assert(
    [] {
        constexpr auto result = 5.0_ms - 2.0_ms;
        return result.get() == 3.0;
    }(),
    "5ms - 2ms = 3ms");

// ---- cross-scale add/sub (normalizes to base) ----

static_assert(
    [] {
        constexpr auto result = 1.0_km + 500.0_m;
        return result.get() == 1500.0;
    }(),
    "1km + 500m = 1500m");

static_assert(
    [] {
        constexpr auto result = 500.0_m + 1.0_km;
        return result.get() == 1500.0;
    }(),
    "500m + 1km = 1500m");

static_assert(
    [] {
        constexpr auto result = 2.0_km - 500.0_m;
        return result.get() == 1500.0;
    }(),
    "2km - 500m = 1500m");

// ---- cross-scale same-tag (different ratios → base) ----

static_assert(
    [] {
        constexpr auto result = 1.0_km + 50000.0_cm;
        return result.get() == 1500.0;
    }(),
    "1km + 50000cm = 1500m");

// ---- scalar multiply/divide (preserve scale) ----

static_assert(
    [] {
        constexpr auto result = 2.0_km * 3.0;
        return result.get() == 6.0;
    }(),
    "2km * 3 = 6km");

static_assert(
    [] {
        constexpr auto result = 3.0 * 2.0_km;
        return result.get() == 6.0;
    }(),
    "3 * 2km = 6km");

static_assert(
    [] {
        constexpr auto result = 6.0_km / 2.0;
        return result.get() == 3.0;
    }(),
    "6km / 2 = 3km");

// ---- cross-scale comparison ----

static_assert(1.0_km == 1000.0_m, "1km == 1000m");
static_assert(2.0_hr == 120.0_min, "2hr == 120min");
static_assert(1.0_km > 999.0_m, "1km > 999m");
static_assert(999.0_m < 1.0_km, "999m < 1km");

// ---- unary negate ----

static_assert((-5.0_km).get() == -5.0, "negate km");
static_assert((-(3.0_ms)).get() == -3.0, "negate ms");

// ---- cross-dimensional via to_base() ----

static_assert(
    [] {
        constexpr auto speed = (100.0_km).to_base() / (2.0_hr).to_base();
        // 100000m / 7200s ≈ 13.888...
        return speed.get() > 13.88 && speed.get() < 13.89;
    }(),
    "100km / 2hr = ~13.89 m/s");

// ---- cross-dimensional multiply (delegates to base) ----

static_assert(
    [] {
        constexpr auto area = 2.0_km * 3.0_km;
        // 2000m * 3000m = 6000000 m²
        return area.get() == 6000000.0;
    }(),
    "2km * 3km = 6000000 m²");

// ---- same-tag division → scalar ----

static_assert(
    [] {
        constexpr auto ratio = 2.0_km / 500.0_m;
        return ratio == 4.0;
    }(),
    "2km / 500m = 4.0 (scalar)");

// ---- default construction ----

static_assert(Kilometers<double>{}.get() == 0.0, "default km is 0");
static_assert(Milliseconds<double>{}.get() == 0.0, "default ms is 0");
// NOLINTEND(readability-magic-numbers,readability-identifier-length)
