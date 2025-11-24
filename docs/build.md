## Building `bearc`
#### Dependencies
- CMake >= 3.20
- Make and/or Ninja
- In my scripts and directions, I use Make for building `bearc` itself, but Ninja for building LLVM.
### Linux/MacOS
```
git clone https://github.com/zachMahan64/bear-lang.git
cd bear-lang && mkdir build && cd build && cmake .. && make
                                                         # if you want BearLang on your path:
cd bearc
pwd                                                      # and copy result
echo "export PATH='result/from/pwd/:\$PATH'" >> ~/.zshrc  # or .bashrc, etc.
source ~/.zshrc                                          # or .bashrc, etc.
bearc -v                                                 # check to ensure it works
```

### Windows with MSYS2 UCRT64
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
cd bearc
pwd                                                       # and copy result
echo "export PATH='result/from/pwd/:\$PATH'" >> ~/.bashrc  
source ~/.bashrc                                          
bearc -v                                                  # check to ensure it works
```

### Building LLVM
```
# starting from project root dir
cd bearc
git clone --branch llvmorg-21.1.6 --depth 1 https://github.com/llvm/llvm-project.git

mkdir llvm-build
cd llvm-build

# tested & working macos install:

cmake -G Ninja ../llvm-project/llvm \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_ENABLE_PROJECTS="clang;lld" \
  -DLLVM_ENABLE_RUNTIMES="" \
  -DLIBCXXABI_USE_LLVM_UNWINDER=OFF \
  -DLLVM_INCLUDE_TESTS=OFF \
  -DLLVM_INCLUDE_DOCS=OFF \
  -DLLVM_INCLUDE_BENCHMARKS=OFF \
  -DLLVM_BUILD_EXAMPLES=OFF \
  -DLLVM_ENABLE_ASSERTIONS=OFF \
  -DLLVM_TARGETS_TO_BUILD="X86;ARM;AArch64" \
  -DCMAKE_INSTALL_PREFIX=../llvm

ninja install

### using example llvm build (fails on macos, haven't tested elsewhere yet)
cmake -G Ninja \
  -C ../llvm-project/clang/cmake/caches/DistributionExample.cmake \
  -DCMAKE_INSTALL_PREFIX=../llvm \
  ../llvm-project/llvm

ninja stage2-distribution
ninja stage2-install-distribution
#^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^#

# strip (optional)
strip bearc/llvm/bin/clang
strip bearc/llvm/bin/clang++
strip bearc/llvm/bin/lld
strip bearc/llvm/bin/opt
strip bearc/llvm/bin/llc
