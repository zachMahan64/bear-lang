#!/usr/bin/env bash
set -euo pipefail

# Usage: ./clean.sh [Debug|Release]
config="${1:-Debug}"

case "$config" in
    Debug|Release) ;;
    *) echo "Invalid config: $config (expected Debug or Release)" >&2; exit 1;;
esac

rm -rf build
cmake -DCMAKE_BUILD_TYPE="$config" -B build
cmake --build build -j"$(nproc)"

