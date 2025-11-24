# BearLang (Work in Progress)
- A modern, C-like language inspired by C++ and Rust.
- BearLang is statically-typed and compiled aiming for simplicity and clarity.
- Right now, I'm working on the frontend but plan on using LLVM for the backend.

## Documentation
- [Project Timeline](/docs/timeline.md)
- [Syntax](docs/syntax.md)
- [References](docs/references.md)

## Setting Up BearLang
#### Build
- See the [build docs](docs/build.md). You can use these scripts:
```
./scripts/llvm-install.sh          # for installing the LLVM distribution
./scripts/clean.sh [Release|Debug] # defaults to debug with no args
```
#### Run 
```
bearc --help          # to see CLI usage
```
