#!/usr/bin/env bash
set -euo pipefail
cmake -B build -DCMAKE_CXX_STANDARD=23 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
