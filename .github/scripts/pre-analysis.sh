#!/usr/bin/env bash
set -euo pipefail
# Configure only: clang-tidy needs compile_commands.json, and the test targets are what put entries in it
cmake -B build -DCMAKE_CXX_STANDARD=23 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_TESTING=ON
