# BearLang (Work in Progress), by Zach Mahan
- A modern, C-like language inspired by C++, Java, and Rust, targeting an in-house VM.
- BearLang is statically-typed and compiled aiming for simplicity and clarity.

## Documentation
- [Project Timeline](/docs/timeline.md)
- [Syntax](docs/references.md)
- [References](docs/references.md)

## Setting Up BearLang
### Build Directions (POSIX)
```
$ git clone https://github.com/zachMahan64/bear-lang.git
$ cd bear-lang && mkdir build && cd build && cmake .. && make
                        # if you want BearLang on your path:
$ pwd                   # and copy result
$ vim ~/.zshrc          # or .bashrc, etc.
                        # add this line into the rc file: export PATH="result/from/pwd/:$PATH" and then :wq
$ source ~/.zshrc       # or .bashrc, etc.
$ bearc -v                # check to ensure it works
```
### Run 
```
$ bearc --help             # to see CLI usage
```
