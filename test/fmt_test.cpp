// NOLINTBEGIN(readability-magic-numbers) — test file
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "strong-types/fmt.hpp"
#include "strong-types/si_literals.hpp"
#include "strong-types/si_scaled_literals.hpp"

using namespace strong_types;
using namespace strong_types::si_literals;
using namespace strong_types::si_scaled_literals;

// ---- Strong (base unit) formatting ----

TEST_CASE("Strong base units format with suffix")
{
    CHECK(fmt::format("{}", unit_t<double, LengthTag>{3.14}) == "3.14 m");
    CHECK(fmt::format("{}", unit_t<double, MassTag>{2.5}) == "2.5 kg");
    CHECK(fmt::format("{}", unit_t<double, TimeTag>{10.0}) == "10 s");
    CHECK(fmt::format("{}", unit_t<double, AreaTag>{4.0}) == "4 m2");
    CHECK(fmt::format("{}", unit_t<double, SpeedTag>{9.8}) == "9.8 m/s");
    CHECK(fmt::format("{}", unit_t<double, AccelerationTag>{9.81}) == "9.81 m/s2");
    CHECK(fmt::format("{}", unit_t<double, ForceTag>{100.0}) == "100 N");
    CHECK(fmt::format("{}", unit_t<double, EnergyTag>{50.0}) == "50 J");
    CHECK(fmt::format("{}", unit_t<double, HertzTag>{60.0}) == "60 Hz");
    CHECK(fmt::format("{}", unit_t<double, CelsiusTag>{22.5}) == "22.5 degC");
    CHECK(fmt::format("{}", unit_t<double, VoltTag>{3.3}) == "3.3 V");
    CHECK(fmt::format("{}", unit_t<double, RadianTag>{1.57}) == "1.57 rad");
    CHECK(fmt::format("{}", unit_t<double, SteradianTag>{6.28}) == "6.28 sr");
}

TEST_CASE("Strong with format spec")
{
    CHECK(fmt::format("{:.2f}", unit_t<double, LengthTag>{3.14159}) == "3.14 m");
    CHECK(fmt::format("{:.1f}", unit_t<double, MassTag>{2.56}) == "2.6 kg");
    CHECK(fmt::format("{:.0f}", unit_t<double, TimeTag>{10.7}) == "11 s");
}

TEST_CASE("Strong with UDL")
{
    CHECK(fmt::format("{}", 5.0_m) == "5 m");
    CHECK(fmt::format("{}", 9.81_mps2) == "9.81 m/s2");
    CHECK(fmt::format("{}", 100.0_N) == "100 N");
}

TEST_CASE("Strong with unknown tag has no suffix")
{
    struct CustomTag;
    CHECK(fmt::format("{}", unit_t<double, CustomTag>{42.0}) == "42");
}

// ---- ScaledUnit formatting ----

TEST_CASE("ScaledUnit formats with scaled suffix")
{
    CHECK(fmt::format("{}", Kilometers<double>{5.0}) == "5 km");
    CHECK(fmt::format("{}", Centimeters<double>{100.0}) == "100 cm");
    CHECK(fmt::format("{}", Millimeters<double>{250.0}) == "250 mm");
    CHECK(fmt::format("{}", Hours<double>{1.5}) == "1.5 hr");
    CHECK(fmt::format("{}", Minutes<double>{30.0}) == "30 min");
    CHECK(fmt::format("{}", Milliseconds<double>{500.0}) == "500 ms");
    CHECK(fmt::format("{}", Microseconds<double>{100.0}) == "100 us");
    CHECK(fmt::format("{}", Grams<double>{250.0}) == "250 g");
    CHECK(fmt::format("{}", Milligrams<double>{50.0}) == "50 mg");
}

TEST_CASE("ScaledUnit with UDL")
{
    CHECK(fmt::format("{}", 5.0_km) == "5 km");
    CHECK(fmt::format("{}", 100.0_cm) == "100 cm");
    CHECK(fmt::format("{}", 10_mm) == "10 mm");
    CHECK(fmt::format("{}", 1.5_hr) == "1.5 hr");
    CHECK(fmt::format("{}", 30.0_min) == "30 min");
    CHECK(fmt::format("{}", 500.0_ms) == "500 ms");
    CHECK(fmt::format("{}", 100.0_us) == "100 us");
    CHECK(fmt::format("{}", 250.0_g) == "250 g");
    CHECK(fmt::format("{}", 50.0_mg) == "50 mg");
}

TEST_CASE("ScaledUnit with format spec")
{
    CHECK(fmt::format("{:.2f}", 3.14159_km) == "3.14 km");
    CHECK(fmt::format("{:.1f}", 2.56_hr) == "2.6 hr");
}

TEST_CASE("ScaledUnit with unknown ratio falls back to tag suffix")
{
    using CustomScale = ScaledUnit<double, LengthTag, std::ratio<100>>;
    CHECK(fmt::format("{}", CustomScale{7.0}) == "7 m");
}

// ---- QuantityPoint formatting ----

TEST_CASE("QuantityPoint formats with suffix")
{
    struct TestOrigin {};
    using QP = QuantityPoint<double, LengthTag, TestOrigin>;
    CHECK(fmt::format("{}", QP{42.5}) == "42.5 m");
    CHECK(fmt::format("{:.1f}", QP{3.14159}) == "3.1 m");
}

TEST_CASE("QuantityPoint with void origin formats same as with origin")
{
    using QP = QuantityPoint<double, LengthTag>;
    CHECK(fmt::format("{}", QP{100.0}) == "100 m");
}

TEST_CASE("QuantityPoint with unknown tag has no suffix")
{
    struct CustomTag;
    using QP = QuantityPoint<double, CustomTag>;
    CHECK(fmt::format("{}", QP{42.0}) == "42");
}

// NOLINTEND(readability-magic-numbers)
