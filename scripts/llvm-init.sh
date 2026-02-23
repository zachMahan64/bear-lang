#!/usr/bin/env bash

# starting from project root dir

./scripts/llvm-install.sh
cd ../../
./scripts/llvm-remove-artifacts.sh
