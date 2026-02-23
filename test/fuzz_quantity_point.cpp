// NOLINTBEGIN(readability-magic-numbers,readability-identifier-length) -- fuzz harness

#include "strong-types/quantity_point.hpp"
#include "strong-types/si_literals.hpp"
#include "strong-types/si_scaled.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>

using namespace strong_types;

struct TestOrigin
{
};
using Point = QuantityPoint<double, LengthTag, TestOrigin>;
using Disp = unit_t<double, LengthTag>;

static bool approx_eq(double a, double b)
{
    if (std::isnan(a) || std::isnan(b) || std::isinf(a) || std::isinf(b))
    {
        return true; // skip degenerate cases, don't trap
    }
    double diff = std::abs(a - b);
    double mag = std::max(1.0, std::max(std::abs(a), std::abs(b)));
    return diff <= mag * 1e-10;
}

// (point + disp) - point == disp
static void check_displacement_recovery(double pv, double dv)
{
    Point pt{pv};
    Disp disp{dv};
    auto shifted = pt + disp;
    auto recovered = shifted - pt;
    if (!approx_eq(recovered.get(), dv))
    {
        __builtin_trap();
    }
}

// (point - disp) + disp == point
static void check_inverse(double pv, double dv)
{
    Point pt{pv};
    Disp disp{dv};
    auto shifted = pt - disp;
    auto back = shifted + disp;
    if (!approx_eq(back.get(), pv))
    {
        __builtin_trap();
    }
}

// point1 - point2 == -(point2 - point1)
static void check_anti_symmetry(double v1, double v2)
{
    Point p1{v1};
    Point p2{v2};
    auto d12 = p1 - p2;
    auto d21 = p2 - p1;
    if (!approx_eq(d12.get(), -d21.get()))
    {
        __builtin_trap();
    }
}

// ScaledUnit + QuantityPoint path
static void check_scaled_add(double pv, double sv)
{
    Point pt{pv};
    Kilometers<double> km{sv};
    auto shifted = pt + km;
    auto expected = pv + sv * 1000.0;
    if (!approx_eq(shifted.get(), expected))
    {
        __builtin_trap();
    }
}

// disp + point == point + disp (commutativity)
static void check_commutative(double pv, double dv)
{
    Point pt{pv};
    Disp disp{dv};
    auto lhs = pt + disp;
    auto rhs = disp + pt;
    if (!approx_eq(lhs.get(), rhs.get()))
    {
        __builtin_trap();
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 16)
    {
        return 0;
    }

    double a{};
    double b{};
    std::memcpy(&a, data, sizeof(a));
    std::memcpy(&b, data + sizeof(a), sizeof(b));

    // Skip NaN/Inf — not interesting for algebraic checks
    if (std::isnan(a) || std::isnan(b) || std::isinf(a) || std::isinf(b))
    {
        return 0;
    }

    // Skip extreme magnitudes where floating-point precision loss makes
    // algebraic invariants (e.g. (p+d)-p == d) trivially fail
    constexpr double magnitude_limit = 1e15;
    if (std::abs(a) > magnitude_limit || std::abs(b) > magnitude_limit)
    {
        return 0;
    }

    check_displacement_recovery(a, b);
    check_inverse(a, b);
    check_anti_symmetry(a, b);
    check_scaled_add(a, b);
    check_commutative(a, b);

    return 0;
}

// NOLINTEND(readability-magic-numbers,readability-identifier-length)
