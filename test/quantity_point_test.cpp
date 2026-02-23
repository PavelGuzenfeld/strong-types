// NOLINTBEGIN(readability-magic-numbers,readability-identifier-length) — test file
#include "strong-types/quantity_point.hpp"
#include "strong-types/si_literals.hpp"
#include "strong-types/si_scaled_literals.hpp"

using namespace strong_types;
using namespace strong_types::si_literals;
using namespace strong_types::si_scaled_literals;

// ---- Origin tags ----

struct MSLOrigin {};
struct AGLOrigin {};

using AltitudeMSL = QuantityPoint<double, LengthTag, MSLOrigin>;
using AltitudeAGL = QuantityPoint<double, LengthTag, AGLOrigin>;
using PositionX = QuantityPoint<double, LengthTag>;

// ---- Default construction ----

static_assert(PositionX{}.get() == 0.0, "default QuantityPoint is zero");
static_assert(AltitudeMSL{}.get() == 0.0, "default MSL altitude is zero");

// ---- Explicit construction ----

static_assert(AltitudeMSL{100.0}.get() == 100.0, "construct MSL altitude");
static_assert(AltitudeAGL{30.0}.get() == 30.0, "construct AGL altitude");

// ---- Comparison operators ----

static_assert(AltitudeMSL{100.0} == AltitudeMSL{100.0}, "equality");
static_assert(AltitudeMSL{100.0} != AltitudeMSL{200.0}, "inequality");
static_assert(AltitudeMSL{100.0} < AltitudeMSL{200.0}, "less than");
static_assert(AltitudeMSL{200.0} > AltitudeMSL{100.0}, "greater than");
static_assert(AltitudeMSL{100.0} <= AltitudeMSL{100.0}, "less or equal");
static_assert(AltitudeMSL{200.0} >= AltitudeMSL{100.0}, "greater or equal");

// ---- Point - Point (same origin) → displacement ----

static_assert(
    [] {
        constexpr AltitudeMSL a{200.0};
        constexpr AltitudeMSL b{150.0};
        constexpr auto diff = a - b;
        static_assert(std::is_same_v<decltype(diff), const unit_t<double, LengthTag>>);
        return diff.get() == 50.0;
    }(),
    "point - point = displacement");

// ---- Point + displacement → Point ----

static_assert(
    [] {
        constexpr AltitudeMSL msl{100.0};
        constexpr auto disp = 30.0_m;
        constexpr auto result = msl + disp;
        static_assert(std::is_same_v<decltype(result), const AltitudeMSL>);
        return result.get() == 130.0;
    }(),
    "point + displacement = point");

// ---- displacement + Point → Point (commutative) ----

static_assert(
    [] {
        constexpr auto disp = 30.0_m;
        constexpr AltitudeMSL msl{100.0};
        constexpr auto result = disp + msl;
        static_assert(std::is_same_v<decltype(result), const AltitudeMSL>);
        return result.get() == 130.0;
    }(),
    "displacement + point = point (commutative)");

// ---- Point - displacement → Point ----

static_assert(
    [] {
        constexpr AltitudeMSL msl{100.0};
        constexpr auto disp = 30.0_m;
        constexpr auto result = msl - disp;
        static_assert(std::is_same_v<decltype(result), const AltitudeMSL>);
        return result.get() == 70.0;
    }(),
    "point - displacement = point");

// ---- Point + ScaledUnit → Point ----

static_assert(
    [] {
        constexpr AltitudeMSL msl{100.0};
        constexpr auto disp = 5.0_km;
        constexpr auto result = msl + disp;
        static_assert(std::is_same_v<decltype(result), const AltitudeMSL>);
        return result.get() == 5100.0;
    }(),
    "point + scaled displacement = point");

// ---- ScaledUnit + Point → Point ----

static_assert(
    [] {
        constexpr auto disp = 5.0_km;
        constexpr AltitudeMSL msl{100.0};
        constexpr auto result = disp + msl;
        static_assert(std::is_same_v<decltype(result), const AltitudeMSL>);
        return result.get() == 5100.0;
    }(),
    "scaled displacement + point = point");

// ---- Point - ScaledUnit → Point ----

static_assert(
    [] {
        constexpr AltitudeMSL msl{10000.0};
        constexpr auto disp = 5.0_km;
        constexpr auto result = msl - disp;
        static_assert(std::is_same_v<decltype(result), const AltitudeMSL>);
        return result.get() == 5000.0;
    }(),
    "point - scaled displacement = point");

// ---- void origin (no origin tag) ----

static_assert(
    [] {
        constexpr PositionX a{10.0};
        constexpr PositionX b{3.0};
        constexpr auto diff = a - b;
        return diff.get() == 7.0;
    }(),
    "void-origin point subtraction works");

// ---- Integer QuantityPoints ----

static_assert(
    [] {
        using IntPoint = QuantityPoint<int, LengthTag>;
        constexpr IntPoint a{100};
        constexpr IntPoint b{30};
        constexpr auto diff = a - b;
        return diff.get() == 70;
    }(),
    "integer QuantityPoint subtraction");

// ---- Compile-time negative test: Point + Point must NOT compile ----
// Uses SFINAE concept check

template <typename A, typename B>
concept CanAdd = requires(A a, B b) { a + b; };

static_assert(!CanAdd<AltitudeMSL, AltitudeMSL>, "point + point must not compile");
static_assert(!CanAdd<AltitudeAGL, AltitudeAGL>, "point + point must not compile (AGL)");

// ---- Compile-time negative test: different-origin subtraction must NOT compile ----

template <typename A, typename B>
concept CanSubtract = requires(A a, B b) { a - b; };

static_assert(!CanSubtract<AltitudeMSL, AltitudeAGL>, "different-origin subtraction must not compile");
static_assert(!CanSubtract<AltitudeAGL, AltitudeMSL>, "different-origin subtraction must not compile (reversed)");

// ---- Detector trait ----

static_assert(is_quantity_point_v<AltitudeMSL>, "detector: AltitudeMSL is a QuantityPoint");
static_assert(is_quantity_point_v<AltitudeAGL>, "detector: AltitudeAGL is a QuantityPoint");
static_assert(!is_quantity_point_v<unit_t<double, LengthTag>>, "detector: unit_t is NOT a QuantityPoint");
static_assert(!is_quantity_point_v<double>, "detector: double is NOT a QuantityPoint");

// NOLINTEND(readability-magic-numbers,readability-identifier-length)
