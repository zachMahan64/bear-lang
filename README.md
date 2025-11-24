# BearLang (Work in Progress)
- A modern, C-like language inspired by C++ and Rust.
- BearLang is statically-typed and compiled aiming for simplicity and clarity.
- Right now, I'm working on the frontend but plan on using LLVM for the backend.

## Documentation
- [Project Timeline](/docs/timeline.md)
- [Syntax](docs/syntax.md)
- [References](docs/references.md)

## Setting Up BearLang
### Build Directions (Linux/MacOS)
```
git clone https://github.com/zachMahan64/bear-lang.git
cd bear-lang && mkdir build && cd build && cmake .. && make
                                                         # if you want BearLang on your path:
pwd                                                      # and copy result
echo "export PATH='result/from/pwd/:\$PATH'" >> ~/.zshrc  # or .bashrc, etc.
source ~/.zshrc                                          # or .bashrc, etc.
bearc -v                                                 # check to ensure it works
```

### Build Directions (Windows with MSYS2 UCRT64)
First, [download MSYS2](https://www.msys2.org/), and then open MSYS2 UCRT64.
```
pacman -Syu
pacman -S git vim     # some essentials you will need if you haven't used MSYS2 before
git clone https://github.com/zachMahan64/bear-lang.git
pacman -S mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-make      \
    mingw-w64-ucrt-x86_64-cmake
cd bear-lang && mkdir build && cd build && cmake -G "MinGW Makefiles" .. && mingw32-make
                                                          # if you want BearLang on your path:
pwd                                                       # and copy result
echo "export PATH='result/from/pwd/:\$PATH'" >> ~/.bashrc  
source ~/.bashrc                                          
bearc -v                                                  # check to ensure it works
```

### Building LLVM
```
# note: adjust paths according to wherever your bear-lang lives
cd bearc
git clone --branch llvmorg-21.1.6 --depth 1 https://github.com/llvm/llvm-project.git
mkdir -p bearc/llvm-build
cd bearc/llvm-build
cmake -G Ninja \
  -C ../llvm-project/clang/cmake/caches/DistributionExample.cmake \
  -DCMAKE_INSTALL_PREFIX=$HOME/dev/bear-lang/bearc/llvm \
  ../llvm-project/llvm
ninja stage2-distribution
ninja stage2-install-distribution

# strip
strip bearc/llvm/bin/clang
strip bearc/llvm/bin/clang++
strip bearc/llvm/bin/lld
strip bearc/llvm/bin/opt
strip bearc/llvm/bin/llc
```

### Run 
```
bearc --help          # to see CLI usage
```
