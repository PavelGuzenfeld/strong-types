# strong-types

[![Release](https://img.shields.io/github/v/release/PavelGuzenfeld/strong-types?label=version&color=blue)](https://github.com/PavelGuzenfeld/strong-types/releases)
[![Build & Test](https://github.com/PavelGuzenfeld/strong-types/actions/workflows/build-test.yml/badge.svg)](https://github.com/PavelGuzenfeld/strong-types/actions/workflows/build-test.yml)

> Strong types for C++23 — use types like `Length`, `Speed`, or `Strong<Vec2, PositionTag>` and get compiler-verified physical semantics.

## Features

- **constexpr-everything** — compile-time math where supported
- **SI units** with trait-based dimensional analysis (length, mass, time, speed, force, energy, power, pressure, etc.)
- **scaled units** — `Kilometers`, `Milliseconds`, `Grams`, `KilometersPerHour`, etc. with compile-time ratio conversions
- **user-defined literals** — `5.0_m`, `9.81_mps2`, `100.0_km`, `36.0_kmh`, `500.0_ms`
- **opt-in `{fmt}` formatting** — `fmt::format("{:.2f}", 3.14_km)` → `"3.14 km"`
- **scalar + vector math** with full STL iterator compatibility (`AlignedArray`)
- **compile-time validation** through `static_assert` tests
- **narrowing protection** on `ScaledUnit` construction (same two-overload pattern as `Strong<T, Tag>`)
- **CI** — GCC 13/14, Clang 17/18, MSVC × Debug/Release

## Comparison with Alternatives

| Feature | **strong-types** | [mp-units](https://github.com/mpusz/mp-units) | [Au](https://github.com/aurora-opensource/au) | [nholthaus/units](https://github.com/nholthaus/units) | [Boost.Units](https://www.boost.org/doc/libs/release/libs/units/) |
|---|---|---|---|---|---|
| C++ standard | **C++23** | C++20 | C++14 | C++14 | C++98 |
| Header-only | yes | no (Conan/vcpkg) | yes | yes (single header) | no (Boost) |
| Dependencies | **zero** | gsl-lite or std | none | none | Boost |
| Approximate LOC | **~1 500** | ~30 000 | ~15 000 | ~12 000 (single header) | ~20 000 |
| constexpr | **everything** | most ops | most ops | partial | no |
| Concepts / `<=>` | yes | yes | no | no | no |
| Custom non-arithmetic T | **yes** (`Strong<Vec2, Tag>`) | no | no | no | no |
| Narrowing protection | **yes** (static_assert) | yes | yes (safe casts) | no | no |
| Dimensional analysis | trait-based, user-extensible | automatic | automatic | automatic | MPL-based |
| Scaled units (km, ms) | yes (`ScaledUnit<T,Tag,Ratio>`) | yes | yes | yes | yes |
| User-defined literals | yes (18 base + 15 scaled) | yes | yes | yes | no |
| `{fmt}` / `std::format` | opt-in `{fmt}` | yes | yes | `<iostream>` | `<iostream>` |
| Chrono interop | yes | yes | yes | yes | no |
| Quantity points / affine | no | yes | yes | no | no |
| Integer overflow safety | narrowing blocked | partial | **best-in-class** | unsafe | no |
| CI matrix (compilers) | GCC/Clang/MSVC | GCC/Clang/MSVC | GCC/Clang/MSVC | GCC/Clang/MSVC | Boost CI |
| Maintained | **active** | **active** (ISO proposal) | **active** | active (3.x) | unmaintained since 2010 |

### When to choose strong-types

- You want **zero-dependency, minimal footprint** — drop a few headers into your project and go
- You need `Strong<T, Tag>` with **non-arithmetic T** (vectors, quaternions, custom math types)
- You prefer **explicit trait rules** you can read and extend over automatic dimension deduction
- Your project already requires **C++23** and you want to leverage concepts, `<=>`, and `constexpr` throughout
- You value **fast compile times** — ~1 500 LOC means negligible overhead

### When to choose something else

- You need **hundreds of units** out of the box (mp-units, Au)
- You need **quantity points / affine types** for temperatures or timestamps (mp-units, Au)
- You need **safe integer math** on embedded targets (Au)
- You are stuck on **C++14/17** (Au, nholthaus)

## Installation

### CMake (FetchContent)

```cmake
include(FetchContent)
FetchContent_Declare(strong-types
    GIT_REPOSITORY https://github.com/PavelGuzenfeld/strong-types.git
    GIT_TAG v0.0.5
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

Then in your project:

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

### Scaled units

```cpp
#include "strong-types/si_scaled.hpp"
#include "strong-types/si_scaled_literals.hpp"

using namespace strong_types;
using namespace strong_types::si_scaled_literals;

constexpr auto d = 5.0_km;
static_assert(d.to_base().get() == 5000.0);  // 5 km = 5000 m

constexpr auto t = 1.0_hr;
static_assert(t.in<std::ratio<60>>().get() == 60.0);  // 1 hr = 60 min

// from_base: convert base unit back to scaled
constexpr auto meters = unit_t<double, LengthTag>{1000.0};
constexpr auto km = from_base<Kilometers<double>>(meters);
static_assert(km.get() == 1.0);  // 1000 m = 1 km

// scale_cast: convert between different scales
constexpr auto hours = Hours<double>{2.0};
constexpr auto mins = scale_cast<Minutes<double>>(hours);
static_assert(mins.get() == 120.0);  // 2 hr = 120 min
```

### Formatting (opt-in, requires `{fmt}`)

```cpp
#include "strong-types/fmt.hpp"

fmt::print("{}\n", 9.81_mps2);      // "9.81 m/s2"
fmt::print("{:.1f}\n", 3.14159_km); // "3.1 km"
fmt::print("{}\n", 500.0_ms);       // "500 ms"
fmt::print("{}\n", 36.0_kmh);       // "36 km/h"
fmt::print("{}\n", 100.0_W);        // "100 W"
```

### Chrono / timespec / timeval interop

```cpp
#include "strong-types/si_chrono.hpp"
#include "strong-types/si_literals.hpp"
#include "strong-types/si_scaled_literals.hpp"

using namespace strong_types;
using namespace strong_types::si_literals;
using namespace strong_types::si_scaled_literals;

// chrono -> strong types
constexpr auto dur = from_chrono(std::chrono::milliseconds(500));
static_assert(dur.get() == 0.5);  // 500ms -> 0.5s

// strong types -> chrono
constexpr auto ms = to_chrono(250.0_ms);
static_assert(ms.count() == 250.0);  // Milliseconds<double> -> chrono ms

// timespec round-trip
constexpr struct timespec ts = {2, 500000000L};
constexpr auto secs = from_timespec(ts);
static_assert(secs.get() == 2.5);
```

### Custom strong types

```cpp
#include "strong-types/strong.hpp"

struct PositionTag {};
using Position = strong_types::Strong<float, PositionTag>;

constexpr Position a{10.0f};
constexpr Position b{5.0f};
static_assert((a + b).get() == 15.0f);
```

## Headers

| Header | Description |
|--------|-------------|
| `strong.hpp` | `Strong<T, Tag>` wrapper, arithmetic ops, type traits |
| `si.hpp` | SI tags (`LengthTag`, `MassTag`, `PowerTag`, ...) and dimensional trait rules |
| `si_literals.hpp` | UDLs for base units (`_m`, `_kg`, `_s`, `_W`, `_Pa`, ...) |
| `si_scaled.hpp` | `ScaledUnit<T, Tag, Ratio>`, `from_base()`, `scale_cast()`, aliases |
| `si_scaled_literals.hpp` | UDLs for scaled units (`_km`, `_cm`, `_mm`, `_hr`, `_ms`, `_kmh`, ...) |
| `si_chrono.hpp` | `constexpr` conversions: `from_chrono`, `to_chrono`, `from_timespec`, `to_timeval`, etc. |
| `fmt.hpp` | Opt-in `fmt::formatter` specializations (requires linking `fmt::fmt`) |
| `aligned_array.hpp` | `AlignedArray<T, N>` for cache-friendly SIMD-like math |

## API Reference

### SI Tags

| Tag | Base Unit | Description |
|-----|-----------|-------------|
| `LengthTag` | m | Length |
| `MassTag` | kg | Mass |
| `TimeTag` | s | Time |
| `AreaTag` | m2 | Area |
| `SpeedTag` | m/s | Speed |
| `AccelerationTag` | m/s2 | Acceleration |
| `ForceTag` | N | Force |
| `EnergyTag` | J | Energy |
| `PowerTag` | W | Power |
| `PressureTag` | Pa | Pressure |
| `HertzTag` | Hz | Frequency |
| `CelsiusTag` | degC | Temperature |
| `VoltTag` | V | Voltage |
| `RadianTag` | rad | Angle |
| `SteradianTag` | sr | Solid angle |
| `AngularVelocityTag` | rad/s | Angular velocity |
| `VolumeTag` | m3 | Volume |
| `DensityTag` | kg/m3 | Density |
| `TorqueTag` | Nm | Torque |

### Dimensional Algebra Rules

| Expression | Result | Rule |
|------------|--------|------|
| `Length / Time` | Speed | `m / s = m/s` |
| `Speed / Time` | Acceleration | `(m/s) / s = m/s2` |
| `Speed * Time` | Length | `(m/s) * s = m` |
| `Mass * Acceleration` | Force | `kg * m/s2 = N` |
| `Force * Length` | Energy | `N * m = J` |
| `Energy / Time` | Power | `J / s = W` |
| `Power * Time` | Energy | `W * s = J` |
| `Force / Area` | Pressure | `N / m2 = Pa` |
| `Pressure * Area` | Force | `Pa * m2 = N` |
| `Radian / Time` | AngularVelocity | `rad / s = rad/s` |
| `AngularVelocity * Time` | Radian | `(rad/s) * s = rad` |
| `Length * Length` | Area | `m * m = m2` |
| `Length * Area` | Volume | `m * m2 = m3` |
| `Volume / Length` | Area | `m3 / m = m2` |
| `Volume / Area` | Length | `m3 / m2 = m` |
| `Mass / Volume` | Density | `kg / m3 = kg/m3` |
| `Density * Volume` | Mass | `(kg/m3) * m3 = kg` |
| `Torque * AngularVelocity` | Power | `Nm * rad/s = W` |
| `Power / AngularVelocity` | Torque | `W / (rad/s) = Nm` |
| `Power / Torque` | AngularVelocity | `W / Nm = rad/s` |
| `1 / Time` | Hertz | `1 / s = Hz` |
| `Tag / Tag` | scalar | same-unit ratio |

All product rules are commutative (`A * B` and `B * A` both work). All same-tag types support `+` and `-`.

### Scaled Unit Aliases

| Alias | Tag | Ratio | UDL |
|-------|-----|-------|-----|
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

### Free Functions

| Function | Description |
|----------|-------------|
| `from_base<TargetScaled>(unit_t<T, Tag>)` | Convert base unit to scaled (e.g. `1000.0_m` -> `Kilometers{1.0}`) |
| `scale_cast<TargetScaled>(ScaledUnit)` | Convert between scales with explicit cast (e.g. `2_hr` -> `Minutes{120}`) |

### Base Unit UDLs (`si_literals`)

`_m`, `_kg`, `_s`, `_m2`, `_mps`, `_mps2`, `_N`, `_J`, `_Hz`, `_degC`, `_V`, `_rad`, `_sr`, `_W`, `_Pa`, `_rps`, `_m3`, `_Nm`

## Tests

```bash
cmake -B build && cmake --build build && ctest --test-dir build
```

The compile-time tests verify all `static_assert` checks pass. The optional `fmt_test` (requires `libfmt`) uses doctest for runtime checks.
