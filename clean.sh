#!/usr/bin/env bash
set -euo pipefail

# cleans build dir
rm -rf build
cmake -DCMAKE_BUILD_TYPE=Debug -B build
cmake --build build "-j$(nproc)"
