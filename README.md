# 🧠 strong-types

> strong types for C++23 that don’t suck. use types like `Distance`, `Velocity`, or even `Strong<Vec2, PositionTag>` and get actual compiler-verified semantics.

## 🔧 features

- **constexpr-everything** — compile-time math where supported
- **type-safe operations** via traits (sum, diff, product, quotient)
- **scalar + vector math** with full STL iterator compatibility
- **custom types** examples like `Vec2`, `AlignedArray`, etc.
- **compile-time validation** through a metric ton of `static_assert`s
- **battle-tested** with `-Werror`, `-pedantic`, `-Wall`, and all the other misery flags

---

## 📦 installation

### with CMake:

```bash
git clone https://github.com/your-name/strong-types.git
cd strong-types
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
sudo make install
```
or, if you're using it in another cmake project:
```cmake
find_package(strong-types REQUIRED)
target_link_libraries(your_target PRIVATE strong-types::strong-types)
```

🧪 usage
include the headers:
```cpp
#include "strong-types/strong.hpp"
#include "strong-types/aligned_array.hpp"
#include "strong-types/si.hpp"
```
create strong types:
```cpp
struct DistanceTag {};
using Distance = strong_types::Strong<float, DistanceTag>;

constexpr Distance a{10.0f};
constexpr Distance b{5.0f};
static_assert((a + b).get() == 15.0f);
```

build physical units the right way:
```cpp
using Length = unit_t<float, LengthTag>;
using Time = unit_t<float, TimeTag>;
using Speed = unit_t<float, SpeedTag>;

constexpr Length d{100.0f};
constexpr Time t{20.0f};
constexpr Speed s = d / t;
static_assert(s.get() == 5.0f);
```
---
🚀 tests
- everything is compile-time verified using static_assert.
  - strong_types_test.cpp
  - strong_aligned_array_test.cpp
  - si_test.cpp
- with full warning flags to ensure you didn’t pull some goofy type casting or miss a virtual destructor somewhere.
---
📚 api overview
 - `Strong<T, Tag>`: strongly typed wrapper over scalars or user-defined types
 - `unit_t<T, UnitTag>`: SI units with trait-based physical semantics
 - `AlignedArray<T, N>`: cache-friendly SIMD-like array for real-time math
 - traits: `tag_sum_result`, `tag_product_result`, `scalar_division_result`, etc.

written by someone who got tired of writing float position and debugging why it was passed instead of velocity.
