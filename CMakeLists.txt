cmake_minimum_required(VERSION 3.20)
project(BearLang)

# Optionally allow building the distribution LLVM
option(BUILD_DISTRIBUTION_LLVM "Build self-contained LLVM distribution inside bearc/llvm" OFF)

add_subdirectory(bearc)
