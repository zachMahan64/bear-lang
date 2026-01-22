#!/usr/bin/env bash
set -euo pipefail

# usage: [Debug|Release]
config="${1:-Debug}"

case "$config" in
    Debug|Release) ;;
    *) echo "Invalid config: $config (expected Debug or Release)" >&2; exit 1;;
esac

rm -rf build
cmake -DCMAKE_BUILD_TYPE="$config" -DNO_LLVM=ON -B build
cmake --build build -j"$(nproc)"
