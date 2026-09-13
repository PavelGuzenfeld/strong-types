# strong-types

[![Release](https://img.shields.io/github/v/release/PavelGuzenfeld/strong-types?label=version&color=blue)](https://github.com/PavelGuzenfeld/strong-types/releases)
[![Build & Test](https://github.com/PavelGuzenfeld/strong-types/actions/workflows/build-test.yml/badge.svg)](https://github.com/PavelGuzenfeld/strong-types/actions/workflows/build-test.yml)
[![OpenSSF Scorecard](https://api.scorecard.dev/projects/github.com/PavelGuzenfeld/strong-types/badge)](https://scorecard.dev/viewer/?uri=github.com/PavelGuzenfeld/strong-types)

Strong types for C++23. `Length`, `Speed`, `Strong<Vec2, PositionTag>`; the compiler checks the physics.

## What you get

- Everything is `constexpr`.
- SI units with dimensional analysis from exponent vectors. `Length / Speed` is `Time` with no rule written for it.
- Scaled units: `Kilometers`, `Milliseconds`, `Grams`, `KilometersPerHour`. Compile-time ratio conversions. An integer representation converts exactly or does not compile.
- Literals: `5.0_m`, `9.81_mps2`, `100.0_km`, `36.0_kmh`, `500.0_ms`.
- Opt-in `{fmt}` support: `fmt::format("{:.2f}", 3.14_km)` gives `"3.14 km"`.
- `AlignedArray<T, N>` for small vector math with STL iterators.
- No silent narrowing. A mismatched constructor argument is a `static_assert`. An integer quantity refuses a fractional scalar. Signed and unsigned integers never meet. `scale_cast` never narrows the representation.
- Quantity points: `QuantityPoint<T, Tag, Origin>` for absolute positions such as MSL altitude or a Celsius reading.
- Checked integer math on `std::expected`: overflow, underflow, division by zero, truncation. The unchecked conversions terminate on overflow instead of wrapping.
- chrono, `timespec` and `timeval` bridges that keep integral representations exact.
- CI on GCC 13/14, Clang 17/18 and MSVC, Debug and Release.

## Compared to mp-units and Au

The measurable difference is compile time. One translation unit computing `speed = length / time`,
gcc 16.0.1, `-O2 -std=c++23`, same host, 2026-09-12:

| | compile time | preprocessed lines | generated code |
|---|---|---|---|
| raw `double` | 0.01 s | 7 | `divsd %xmm1, %xmm0; ret` |
| **strong-types** (`si.hpp`) | 0.03 s | 7,659 | identical |
| [mp-units](https://github.com/mpusz/mp-units) (`si.h` + `isq.h`) | 3.07 s | 145,896 | identical |

The library is about 2,000 lines of headers and depends only on the standard library.

It does not have hundreds of predefined units, unit symbols, ISQ quantity kinds, unit-aware math
functions, C++20 or C++14 support, or a standardisation track. For those, use
[mp-units](https://github.com/mpusz/mp-units) or [Au](https://github.com/aurora-opensource/au).

Pick strong-types when compile time matters and you need a handful of quantities, when you wrap your
own vector or matrix type and want only the operators it has, or when you want a model you can read
in one sitting. Pick something else when you need a catalogue of units, or you are below C++23.

## Installation

### CMake (FetchContent)

```cmake
include(FetchContent)
FetchContent_Declare(strong-types
    GIT_REPOSITORY https://github.com/PavelGuzenfeld/strong-types.git
    GIT_TAG v1.2.3
)
FetchContent_MakeAvailable(strong-types)

target_link_libraries(your_target PRIVATE strong-types)
```

### System install

```bash
git clone https://github.com/PavelGuzenfeld/strong-types.git
cd strong-types
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

```cmake
find_package(strong-types REQUIRED)
target_link_libraries(your_target PRIVATE strong-types::strong-types)
```

## Usage

### Base SI units

```cpp
#include "strong-types/si.hpp"
#include "strong-types/si_literals.hpp"

using namespace strong_types;
using namespace strong_types::si_literals;

constexpr auto distance = 100.0_m;
constexpr auto time = 20.0_s;
constexpr auto speed = distance / time;
static_assert(speed.get() == 5.0);  // Speed = Length / Time
```

Different representations promote to their `std::common_type_t`. `unit_t<float, LengthTag>` plus
`unit_t<double, LengthTag>` is `unit_t<double, LengthTag>` in either order.

Signed and unsigned integers do not mix. `unit_t<int, LengthTag> + unit_t<unsigned, LengthTag>` does
not compile; the usual conversions would turn `-2 + 1u` into 4294967295. The same rule covers a scalar:
`unit_t<unsigned, LengthTag>{3} * -1` does not compile either, since `-1` is a signed `int` that would
wrap the unsigned rep to 4294967293. Scale an unsigned quantity with an unsigned literal (`* 2u`).

An integer quantity refuses a fractional scalar. `unit_t<int, LengthTag>{5} * 0.5` does not compile.
Cast to a floating representation first.

### Scaled units

```cpp
#include "strong-types/si_scaled.hpp"
#include "strong-types/si_scaled_literals.hpp"

using namespace strong_types;
using namespace strong_types::si_scaled_literals;

constexpr auto d = 5.0_km;
static_assert(d.to_base().get() == 5000.0);

constexpr auto t = 1.0_hr;
static_assert(t.in<std::ratio<60>>().get() == 60.0);

constexpr auto meters = unit_t<double, LengthTag>{1000.0};
static_assert(scale_cast<Kilometers<double>>(meters).get() == 1.0);

constexpr auto hours = Hours<double>{2.0};
static_assert(scale_cast<Minutes<double>>(hours).get() == 120.0);
```

An integer representation converts exactly or not at all. `scale_cast` may widen the representation
but never narrow it.

```cpp
static_assert(Kilometers<int>{5}.to_base().get() == 5000);
static_assert(scale_cast<Millimeters<long long>>(unit_t<int, LengthTag>{3}).get() == 3000);
// Grams<int>{500}.to_base();                                      // error: 0.5 kg is not an int
// Millimeters<int>{1} + Kilometers<int>{1};                       // error: no exact meeting point
// scale_cast<Kilometers<int>>(unit_t<double, LengthTag>{1500.0}); // error: narrows the representation
```

A conversion that overflows the representation fails constant evaluation, or calls `std::terminate`
at runtime. `safe_to_base` and `safe_scale_cast` are the checked alternatives.

### Formatting (opt-in, needs `{fmt}`)

```cpp
#include "strong-types/fmt.hpp"

fmt::print("{}\n", 9.81_mps2);      // "9.81 m/s2"
fmt::print("{:.1f}\n", 3.14159_km); // "3.1 km"
fmt::print("{}\n", 500.0_ms);       // "500 ms"
fmt::print("{}\n", 36.0_kmh);       // "36 km/h"
fmt::print("{}\n", 100.0_W);        // "100 W"
```

### chrono, timespec, timeval

```cpp
#include "strong-types/si_chrono.hpp"
#include "strong-types/si_literals.hpp"
#include "strong-types/si_scaled_literals.hpp"

using namespace strong_types;
using namespace strong_types::si_literals;
using namespace strong_types::si_scaled_literals;

constexpr auto dur = from_chrono(std::chrono::milliseconds(500));
static_assert(dur.get() == 0.5);  // always double seconds

constexpr auto us = from_chrono_as<std::micro>(std::chrono::milliseconds(10));
static_assert(us.get() == 10000);  // integral rep kept; exact or refused

constexpr auto ms = to_chrono(250.0_ms);
static_assert(ms.count() == 250.0);  // duration<double, milli>

constexpr auto ns = to_chrono(Nanoseconds<std::int64_t>{7});
static_assert(ns.count() == 7);  // integral rep stays integral

constexpr struct timespec ts = {1'700'000'000, 123'456'789L};
static_assert(from_timespec_as_ns(ts).get() == 1'700'000'000'123'456'789);
static_assert(to_timespec(from_timespec_as_ns(ts)).tv_nsec == 123'456'789L);
```

`from_chrono`, `from_timespec` and `from_timeval` return double seconds. At epoch magnitude a double
cannot hold the low nanoseconds; use `from_chrono_as`, `from_timespec_as_ns` or `from_timeval_as_us`
when that matters.

Every other bridge keeps an integral representation integral. `to_timespec` and `to_timeval` accept
any time unit, round to the nearest nanosecond or microsecond, and keep `tv_nsec` in `[0, 1e9)` for
negative times.

### Custom strong types

```cpp
#include "strong-types/strong.hpp"

struct PositionTag {};
using Position = strong_types::Strong<float, PositionTag>;

constexpr Position a{10.0f};
constexpr Position b{5.0f};
static_assert((a + b).get() == 15.0f);
```

`T` does not have to be arithmetic. Each operator is constrained on the expression it performs. A
`Strong<Vec3, PositionTag>` has `+`, `-`, scalar `*` and `/`, and `==` if `Vec3` does. `a * b` or
`a < b` fails at the call site when `Vec3` has no such operator.

### Cross-tag arithmetic for domain types

SI tags derive their rules from dimension exponents. A custom tag opts in by specializing the tag
traits:

```cpp
struct HullPointsTag {};
struct DamagePointsTag {};

using HullPoints = strong_types::Strong<double, HullPointsTag>;
using DamagePoints = strong_types::Strong<double, DamagePointsTag>;

template<>
struct strong_types::tag_difference_result<HullPointsTag, DamagePointsTag>
{
    using type = HullPointsTag;
};

HullPoints hp{100.0};
DamagePoints dmg{30.0};
HullPoints remaining = hp - dmg;  // HullPoints{70.0}
```

Without a rule, arithmetic between different tags is a constraint failure, so `requires` and
`!CanAdd<A, B>`-style tests can observe it.

### Widening integer construction

`Strong`, `ScaledUnit` and `QuantityPoint` share one rule, `WideningIntegral<From, To>`: an integer
representation accepts any integer type of the same or smaller size without a cast. `scale_cast`
applies the same rule when it changes the representation.

```cpp
struct MyTag {};
using MyId = strong_types::Strong<uint64_t, MyTag>;

MyId a{42};            // int widens to uint64_t
MyId b{42u};           // unsigned widens
MyId c{uint64_t{42}};  // exact match
MyId d{};              // value-initialised to 0
```

`double` to `int` and `int64_t` to `int32_t` are rejected at compile time.

### Quantity points

```cpp
#include "strong-types/quantity_point.hpp"
#include "strong-types/si_literals.hpp"

using namespace strong_types;
using namespace strong_types::si_literals;

struct MSLOrigin {};
struct AGLOrigin {};
using AltitudeMSL = QuantityPoint<double, LengthTag, MSLOrigin>;
using AltitudeAGL = QuantityPoint<double, LengthTag, AGLOrigin>;

constexpr AltitudeMSL msl{100.0};
constexpr AltitudeMSL shifted = msl + 30.0_m;  // point + displacement
static_assert(shifted.get() == 130.0);

constexpr auto diff = shifted - msl;  // point - point = displacement
static_assert(diff.get() == 30.0);

// msl + AltitudeAGL{50.0};  // error: different origins
// msl + shifted;            // error: point + point
```

`Celsius<T>` is a `QuantityPoint` whose difference type is the kelvin interval
`unit_t<T, TemperatureTag>`:

```cpp
constexpr auto delta = 30.0_degC - 20.0_degC;   // unit_t<double, TemperatureTag>, 10 K
constexpr auto warmer = 20.0_degC + 5.0_K;      // Celsius<double>, 25 degC
// 20.0_degC + 30.0_degC;                       // error: point + point
```

### Safe integer math

```cpp
#include "strong-types/safe_math.hpp"
#include "strong-types/si_scaled.hpp"

using namespace strong_types;

constexpr auto result = safe_multiply(1000000, 1000000);
static_assert(!result.has_value());  // overflow

constexpr ScaledUnit<int, LengthTag, std::kilo> km{3000000};
static_assert(safe_to_base(km).error() == ArithmeticErrc::overflow);

constexpr ScaledUnit<int, LengthTag, std::kilo> km5{5};
static_assert(safe_to_base(km5).value().get() == 5000);
```

All of these return `std::expected<T, ArithmeticErrc>`.

A ratio constant that does not fit the representation is a compile error, not a wrapped value.
`safe_to_base(Days<std::int8_t>{1})` does not compile; 86400 is not an `int8_t`. `safe_scale_cast`
checks the representation conversion too and returns `overflow` when the value does not fit the
target. `safe_multiply` uses `__builtin_mul_overflow` where the compiler has it.

## Headers

| Header | Contents |
|--------|----------|
| `strong.hpp` | `Strong<T, Tag>`, arithmetic operators, result traits |
| `si.hpp` | SI tags and the dimension algebra |
| `si_literals.hpp` | Literals for base units: `_m`, `_kg`, `_s`, `_W`, `_Pa`, ... |
| `si_scaled.hpp` | `ScaledUnit<T, Tag, Ratio>`, `scale_cast`, aliases |
| `si_scaled_literals.hpp` | Literals for scaled units: `_km`, `_cm`, `_mm`, `_hr`, `_ms`, `_kmh`, ... |
| `si_chrono.hpp` | chrono, `timespec` and `timeval` conversions |
| `quantity_point.hpp` | `QuantityPoint<T, Tag, Origin>` |
| `safe_math.hpp` | `safe_multiply`, `safe_add`, `safe_subtract`, `safe_divide` |
| `fmt.hpp` | `fmt::formatter` specializations, needs `fmt::fmt` |
| `aligned_array.hpp` | `AlignedArray<T, N>` |

## Reference

### SI tags

| Tag | Unit | `Dim<L, M, T, I, Θ, N, J, A>` |
|-----|------|-------------------------------|
| `LengthTag` | m | `1` |
| `MassTag` | kg | `0, 1` |
| `TimeTag` | s | `0, 0, 1` |
| `AreaTag` | m2 | `2` |
| `VolumeTag` | m3 | `3` |
| `SpeedTag` | m/s | `1, 0, -1` |
| `AccelerationTag` | m/s2 | `1, 0, -2` |
| `ForceTag` | N | `1, 1, -2` |
| `PressureTag` | Pa | `-1, 1, -2` |
| `EnergyTag` | J | `2, 1, -2` |
| `PowerTag` | W | `2, 1, -3` |
| `HertzTag` | Hz | `0, 0, -1` |
| `DensityTag` | kg/m3 | `-3, 1` |
| `VoltTag` | V | `2, 1, -3, -1` |
| `TemperatureTag` | K (interval) | `0, 0, 0, 0, 1` |
| `RadianTag` | rad | `0, 0, 0, 0, 0, 0, 0, 1` |
| `SteradianTag` | sr | `0, 0, 0, 0, 0, 0, 0, 2` |
| `AngularVelocityTag` | rad/s | `0, 0, -1, 0, 0, 0, 0, 1` |
| `TorqueTag` | Nm (J/rad) | `2, 1, -2, 0, 0, 0, 0, -1` |

### Dimension algebra

Every SI tag carries a `Dim<L, M, T, I, Θ, N, J, A>` exponent vector: length, mass, time, current,
temperature, amount, luminous intensity, plane angle. `*` adds exponents, `/` subtracts them, and the
result is the tag registered for that dimension.

| Expression | Result |
|------------|--------|
| `Length / Speed` | Time |
| `Force / Mass` | Acceleration |
| `Energy / Time` | Power |
| `Radian / Time` | AngularVelocity |
| `Speed * Speed` | `unit_t<T, Dim<2, 0, -2>>`, unnamed; `* Mass` composes on to Energy |
| `Hertz * Time` | bare `T` |
| `Length / Length` | bare `T` |
| `1 / Time` | Hertz |

Angle is a dimension. `Hertz` (s⁻¹) and `AngularVelocity` (rad·s⁻¹) are distinct types, and `Torque`
is J/rad rather than a second name for Energy. `+` and `-` require the same tag, so `Energy + Torque`
does not compile.

A tag without a dimension uses the explicit trait rules from "Cross-tag arithmetic" above. Two tags
with the same `Dim` stay distinct types.

### Scaled unit aliases

| Alias | Tag | Ratio | Literal |
|-------|-----|-------|---------|
| `Micrometers<T>` | Length | `micro` | `_um` |
| `Millimeters<T>` | Length | `milli` | `_mm` |
| `Centimeters<T>` | Length | `centi` | `_cm` |
| `Kilometers<T>` | Length | `kilo` | `_km` |
| `Nanoseconds<T>` | Time | `nano` | `_ns` |
| `Microseconds<T>` | Time | `micro` | `_us` |
| `Milliseconds<T>` | Time | `milli` | `_ms` |
| `Minutes<T>` | Time | `ratio<60>` | `_min` |
| `Hours<T>` | Time | `ratio<3600>` | `_hr` |
| `Days<T>` | Time | `ratio<86400>` | `_d` |
| `Weeks<T>` | Time | `ratio<604800>` | `_wk` |
| `Milligrams<T>` | Mass | `ratio<1,1000000>` | `_mg` |
| `Grams<T>` | Mass | `ratio<1,1000>` | `_g` |
| `Tons<T>` | Mass | `kilo` | `_t` |
| `KilometersPerHour<T>` | Speed | `ratio<5,18>` | `_kmh` |

### Conversions

| Function | Result |
|----------|--------|
| `scale_cast<Target>(unit_t<T, Tag>)` | Base to scaled. Integers exact or refused; representation may widen, never narrow |
| `scale_cast<Target>(ScaledUnit)` | Scale to scale, same rules |
| `from_chrono(duration)` | `unit_t<double, TimeTag>` seconds |
| `from_chrono_as<Ratio>(duration)` | `ScaledUnit<rep, TimeTag, Ratio>`; integral rep exact or refused |
| `to_chrono(u)`, `to_chrono_as<Dur>(u)` | `chrono::duration`; integral rep stays integral |
| `from_timespec(ts)`, `from_timeval(tv)` | `unit_t<double, TimeTag>` seconds |
| `from_timespec_as_ns(ts)`, `from_timeval_as_us(tv)` | `Nanoseconds<int64_t>`, `Microseconds<int64_t>`, exact |
| `to_timespec(u)`, `to_timeval(u)` | Nearest ns / us, `tv_nsec` in `[0, 1e9)` |

### Checked math

All return `std::expected<T, ArithmeticErrc>`.

| Function | Checks |
|----------|--------|
| `safe_multiply(T, T)`, `safe_add(T, T)`, `safe_subtract(T, T)` | integer overflow and underflow |
| `safe_divide(T, T)` | division by zero, `INT_MIN / -1`, non-finite float result |
| `safe_to_base(ScaledUnit<int, ...>)` | overflow and truncation |
| `safe_scale_cast<Target>(unit_t<int, ...>)` | overflow, truncation, representation fit |
| `safe_scale_cast<Target>(ScaledUnit<int, ...>)` | overflow, truncation, representation fit |

### Base unit literals

`_m`, `_kg`, `_s`, `_m2`, `_mps`, `_mps2`, `_N`, `_J`, `_Hz`, `_K`, `_V`, `_rad`, `_sr`, `_W`, `_Pa`, `_rps`, `_m3`, `_Nm`

`_degC` is a `Celsius<double>` quantity point, not an interval.

## Tests

Tests are off by default so FetchContent consumers do not build them:

```bash
cmake -B build -DBUILD_TESTING=ON && cmake --build build && ctest --test-dir build
```

The tests are `static_assert`s; a successful build is a pass. `fmt_test` needs `libfmt` and runs under
doctest.

Fuzzing needs Clang:

```bash
cmake -B build -DBUILD_FUZZING=ON && cmake --build build
./build/fuzz_safe_math corpus/safe_math -max_total_time=60
./build/fuzz_quantity_point corpus/quantity_point -max_total_time=60
```

---

[Design notes on pavelguzenfeld.com](https://pavelguzenfeld.com/projects/strong-types/)
