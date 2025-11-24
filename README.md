# BearLang (Work in Progress)
- A modern, C-like language inspired by C++ and Rust.
- BearLang is statically-typed and compiled aiming for simplicity and clarity.
- Right now, I'm working on the frontend but plan on using LLVM for the backend.

## Documentation
- [Project Timeline](/docs/timeline.md)
- [Syntax](docs/syntax.md)
- [References](docs/references.md)

## Set up
#### Build
- See the [build docs](docs/build.md).
- With git, gcc or clang, CMake, and Ninja installed, you can use these scripts: 
```
./scripts/llvm-install.sh          # for installing the LLVM distribution
./scripts/clean.sh [Release|Debug] # builds bearc; defaults to Debug with no args
```
#### Run 
```
bearc --help          # to see CLI usage
```
