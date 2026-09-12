#!/usr/bin/env bash
set -euo pipefail
# .clang-tidy passes -stdlib=libc++; images built before the Dockerfile added it need this
dpkg -s libc++-18-dev >/dev/null 2>&1 || (apt-get update -qq && apt-get install -y -qq libc++-18-dev libc++abi-18-dev)
cmake -B build -DCMAKE_CXX_STANDARD=23 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_TESTING=ON
cmake --build build
