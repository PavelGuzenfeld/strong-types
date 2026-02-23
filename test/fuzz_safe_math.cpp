// NOLINTBEGIN(readability-magic-numbers,readability-identifier-length) -- fuzz harness

#include "strong-types/safe_math.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

using strong_types::ArithmeticErrc;
using strong_types::safe_add;
using strong_types::safe_divide;
using strong_types::safe_multiply;
using strong_types::safe_subtract;

// Widen to int64 to compute the "true" result for oracle checks
static void check_multiply(int32_t a, int32_t b)
{
    auto result = safe_multiply(a, b);
    auto wide = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    bool fits = wide >= std::numeric_limits<int32_t>::min() && wide <= std::numeric_limits<int32_t>::max();

    if (result.has_value())
    {
        // Must fit AND match
        if (!fits || static_cast<int64_t>(result.value()) != wide)
        {
            __builtin_trap();
        }
    }
    else
    {
        // Must NOT fit
        if (fits)
        {
            __builtin_trap();
        }
    }
}

static void check_add(int32_t a, int32_t b)
{
    auto result = safe_add(a, b);
    auto wide = static_cast<int64_t>(a) + static_cast<int64_t>(b);
    bool fits = wide >= std::numeric_limits<int32_t>::min() && wide <= std::numeric_limits<int32_t>::max();

    if (result.has_value())
    {
        if (!fits || static_cast<int64_t>(result.value()) != wide)
        {
            __builtin_trap();
        }
    }
    else
    {
        if (fits)
        {
            __builtin_trap();
        }
    }
}

static void check_subtract(int32_t a, int32_t b)
{
    auto result = safe_subtract(a, b);
    auto wide = static_cast<int64_t>(a) - static_cast<int64_t>(b);
    bool fits = wide >= std::numeric_limits<int32_t>::min() && wide <= std::numeric_limits<int32_t>::max();

    if (result.has_value())
    {
        if (!fits || static_cast<int64_t>(result.value()) != wide)
        {
            __builtin_trap();
        }
    }
    else
    {
        if (fits)
        {
            __builtin_trap();
        }
    }
}

static void check_divide(int32_t a, int32_t b)
{
    auto result = safe_divide(a, b);

    if (b == 0)
    {
        // Must report divide_by_zero
        if (result.has_value() || result.error() != ArithmeticErrc::divide_by_zero)
        {
            __builtin_trap();
        }
        return;
    }

    // INT32_MIN / -1 overflows
    bool overflows = (a == std::numeric_limits<int32_t>::min() && b == -1);
    if (overflows)
    {
        if (result.has_value())
        {
            __builtin_trap();
        }
        return;
    }

    // Normal case — must succeed and match
    auto expected = a / b;
    if (!result.has_value() || result.value() != expected)
    {
        __builtin_trap();
    }
}

static void check_scaled_round_trip(int32_t val)
{
    // safe_to_base(ScaledUnit<int, LengthTag, kilo>) then safe_from_base back
    using Km = strong_types::ScaledUnit<int32_t, strong_types::LengthTag, std::kilo>;
    Km km{val};

    auto base = strong_types::safe_to_base(km);
    if (!base.has_value())
    {
        // Overflow on to_base is expected for large values — just verify the wide math
        auto wide = static_cast<int64_t>(val) * 1000;
        bool fits = wide >= std::numeric_limits<int32_t>::min() && wide <= std::numeric_limits<int32_t>::max();
        if (fits)
        {
            __builtin_trap(); // Should have succeeded
        }
        return;
    }

    // Round-trip: base -> km should recover the original value
    auto back = strong_types::safe_from_base<Km>(base.value());
    if (!back.has_value() || back.value().get() != val)
    {
        __builtin_trap();
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 8)
    {
        return 0;
    }

    int32_t a{};
    int32_t b{};
    std::memcpy(&a, data, sizeof(a));
    std::memcpy(&b, data + sizeof(a), sizeof(b));

    check_multiply(a, b);
    check_add(a, b);
    check_subtract(a, b);
    check_divide(a, b);
    check_scaled_round_trip(a);

    return 0;
}

// NOLINTEND(readability-magic-numbers,readability-identifier-length)
