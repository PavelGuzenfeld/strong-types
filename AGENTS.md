# Agent Instructions — Quality Standard

## Quality Standard

This project uses [diff-aware quality workflows](https://github.com/PavelGuzenfeld/standard) for CI.
Only changed files are checked — but all new and modified code must pass.

### Always Enforced

- **clang-tidy** — clang-analyzer, cppcoreguidelines, modernize, bugprone, performance, readability
- **cppcheck** — bug and style checking with project-specific suppressions

### Opt-in (enabled in this project)

- **clang-format** — C++23, 120-column, 4-space indent, Allman braces
- **File naming** — snake_case for all files and directories
- **Identifier naming** — snake_case functions/variables, PascalCase types, trailing `_` for private members

## Project Overview

Header-only C++23 library providing compile-time strongly-typed wrappers with trait-based dimensional analysis.

- **Namespace**: `strong_types`
- **Package name**: `strong-types` (hyphenated, exception in `naming_exceptions.txt`)
- **Headers**: `include/strong-types/strong.hpp`, `si.hpp`, `aligned_array.hpp`
- **Tests**: `test/` — compile-time verification via `static_assert` (no runtime test framework)
- **Build**: CMake, `CMAKE_CXX_STANDARD 23`, interface library

## C++ Conventions

### Identifier Naming

| Element | Convention | Example |
|---------|-----------|---------|
| Functions / methods | `snake_case` | `compute_heading()` |
| Variables / parameters | `snake_case` | `max_altitude` |
| Types / classes / structs | `PascalCase` | `FlightController` |
| Private members | `snake_case_` (trailing underscore) | `config_`, `state_` |
| Constants / enums | `UPPER_CASE` | `MAX_RETRIES` |
| Namespaces | `snake_case` | `strong_types` |

### Include Convention

```cpp
#pragma once                          // not #ifndef guards
#include <system_headers>             // standard library first
#include "project/package_header.hpp" // project headers second
```

Minimize includes. Forward-declare where possible.

## Testing

Tests use `static_assert` for compile-time verification. The test executable `strong-types_test` builds from all test files in `test/`. A successful build means all tests pass.

```bash
cmake -B build -DCMAKE_CXX_STANDARD=23
cmake --build build
```

## Code Formatting

### clang-format Settings

- Standard: C++23
- Column limit: 120
- Indent: 4 spaces
- Braces: Allman (opening brace on new line)
- No bin-packing of arguments

### clang-tidy Checks

Active check groups: `clang-analyzer-*`, `cppcoreguidelines-*`, `modernize-*`, `bugprone-*`, `performance-*`, `readability-*`

## Local Verification

Run these before pushing to avoid CI failures:

```bash
# clang-tidy on changed files
./scripts/diff-clang-tidy.sh origin/main build "cpp hpp h"

# cppcheck on changed files
./scripts/diff-cppcheck.sh origin/main

# clang-format on changed files
./scripts/diff-clang-format.sh origin/main "cpp hpp h"

# file naming check
./scripts/diff-file-naming.sh origin/main naming_exceptions.txt
```

## Git & PR Rules

- **No AI attribution** — never add "Generated with Claude Code", "Co-Authored-By", or similar AI-generated footers to commit messages, PR descriptions, or any content
- **Conventional commits** — use `feat:`, `fix:`, `feat!:`, `BREAKING CHANGE:` prefixes (drives auto-release versioning)
- **Versioning** — first release is always `v0.0.1`, see [versioning rules](https://github.com/PavelGuzenfeld/standard/blob/main/docs/VERSIONING.md)

## Customization

### Suppressing cppcheck Warnings

Add to `cppcheck.suppress`:

```
// Suppress specific check for a file
unusedFunction:src/legacy_module.cpp

// Suppress globally
shadowVariable
```

### Overriding clang-tidy Checks

Edit `.clang-tidy` in your repo root. The CI uses your config when present.

To disable a specific check:

```yaml
Checks: >-
  ...,
  -modernize-use-trailing-return-type
```
