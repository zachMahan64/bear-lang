#!/usr/bin/env bash
set -euo pipefail

# usage: ./run-tests.sh [Debug|Release]
config="${1:-Debug}"

case "$config" in
    Debug|Release) ;;
    *)
        echo "Invalid config: $config (expected Debug or Release)" >&2
        exit 1
        ;;
esac

rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE="$config" -DTEST=ON -DNO_LLVM=ON
cmake --build build -j"$(nproc)"
echo "Running tests..."
./build/bearc/bearc

