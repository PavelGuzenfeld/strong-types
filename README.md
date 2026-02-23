# strong-types

[![Release](https://img.shields.io/github/v/release/PavelGuzenfeld/strong-types?label=version&color=blue)](https://github.com/PavelGuzenfeld/strong-types/releases)

> Strong types for C++23 — use types like `Length`, `Speed`, or `Strong<Vec2, PositionTag>` and get compiler-verified physical semantics.

## Features

- **constexpr-everything** — compile-time math where supported
- **SI units** with trait-based dimensional analysis (length, mass, time, speed, force, energy, etc.)
- **scaled units** — `Kilometers`, `Milliseconds`, `Grams`, etc. with compile-time ratio conversions
- **user-defined literals** — `5.0_m`, `9.81_mps2`, `100.0_km`, `500.0_ms`
- **opt-in `{fmt}` formatting** — `fmt::format("{:.2f}", 3.14_km)` → `"3.14 km"`
- **scalar + vector math** with full STL iterator compatibility (`AlignedArray`)
- **compile-time validation** through `static_assert` tests
- **strict warnings** — `-Werror`, `-pedantic`, `-Wall`, `-Wconversion`, and more

## Installation

### CMake (FetchContent)

```cmake
include(FetchContent)
FetchContent_Declare(strong-types
    GIT_REPOSITORY https://github.com/PavelGuzenfeld/strong-types.git
    GIT_TAG v0.0.4
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

using namespace strong_types::si_scaled_literals;

constexpr auto d = 5.0_km;
static_assert(d.to_base().get() == 5000.0);  // 5 km = 5000 m

constexpr auto t = 1.0_hr;
static_assert(t.in<std::ratio<60>>().get() == 60.0);  // 1 hr = 60 min
```

### Formatting (opt-in, requires `{fmt}`)

```cpp
#include "strong-types/fmt.hpp"

fmt::print("{}\n", 9.81_mps2);      // "9.81 m/s2"
fmt::print("{:.1f}\n", 3.14159_km); // "3.1 km"
fmt::print("{}\n", 500.0_ms);       // "500 ms"
```

### Chrono / timespec / timeval interop

```cpp
#include "strong-types/si_chrono.hpp"
#include "strong-types/si_literals.hpp"
#include "strong-types/si_scaled_literals.hpp"

using namespace strong_types;
using namespace strong_types::si_literals;
using namespace strong_types::si_scaled_literals;

// chrono → strong types
constexpr auto dur = from_chrono(std::chrono::milliseconds(500));
static_assert(dur.get() == 0.5);  // 500ms → 0.5s

// strong types → chrono
constexpr auto ms = to_chrono(250.0_ms);
static_assert(ms.count() == 250.0);  // Milliseconds<double> → chrono ms

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
| `si.hpp` | SI tags (`LengthTag`, `MassTag`, ...) and dimensional trait rules |
| `si_literals.hpp` | UDLs for base units (`_m`, `_kg`, `_s`, `_mps`, `_N`, ...) |
| `si_scaled.hpp` | `ScaledUnit<T, Tag, Ratio>` with aliases (`Kilometers`, `Milliseconds`, ...) |
| `si_scaled_literals.hpp` | UDLs for scaled units (`_km`, `_cm`, `_mm`, `_hr`, `_ms`, `_ns`, ...) |
| `si_chrono.hpp` | `constexpr` conversions: `from_chrono`, `to_chrono`, `from_timespec`, `to_timeval`, etc. |
| `fmt.hpp` | Opt-in `fmt::formatter` specializations (requires linking `fmt::fmt`) |
| `aligned_array.hpp` | `AlignedArray<T, N>` for cache-friendly SIMD-like math |

## Tests

```bash
cmake -B build && cmake --build build
./build/strong-types_test        # compile-time static_assert tests
./build/strong-types_fmt_test    # runtime fmt tests (requires libfmt)
```
