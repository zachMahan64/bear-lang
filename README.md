# bear-lang (Work in Progress)
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
./scripts/clean-all.sh [Release|Debug] # builds bearc; defaults to Debug with no args
```
#### Run 
```
bearc --help          # to see CLI usage
```
#### Preview
- Heres a preview of my AST pretty-printer as of 20251219:
```
// tests/11.br

fn main() {
    foo();
}

fn foo() -> i32 { 
    i32 a = 2;
    a += 4;
    b = 4;
    c <- b;
    return c;
}

fn bar() {
    waste_effort();
    return;
}
```
```
file 'tests/11.br': {
function declaration: {
|   `fn`,
|   `main`,
|   `(`,
|   `)`,
|   block: {
|   |   `{`,
|   |   expression-statement: {
|   |   |   function call: {
|   |   |   |   identifer: `foo`,
|   |   |   |   `(`,
|   |   |   |   `)`,
|   |   |   },
|   |   `;`,
|   |   }
|   |   `}`,
|   }
}
function declaration: {
|   `fn`,
|   `foo`,
|   `(`,
|   `)`,
|   `->`,
|   base type: {
|   |   `i32`,
|   }
|   block: {
|   |   `{`,
|   |   variable initialization: {
|   |   |   base type: {
|   |   |   |   `i32`,
|   |   |   }
|   |   |   name: `a`,
|   |   |   `=`,
|   |   |   literal (int_lit): `2`,
|   |   `;`,
|   |   }
|   |   expression-statement: {
|   |   |   binary op: {
|   |   |   |   identifer: `a`,
|   |   |   |   `+=`,
|   |   |   |   literal (int_lit): `4`,
|   |   |   },
|   |   `;`,
|   |   }
|   |   expression-statement: {
|   |   |   binary op: {
|   |   |   |   identifer: `b`,
|   |   |   |   `=`,
|   |   |   |   literal (int_lit): `4`,
|   |   |   },
|   |   `;`,
|   |   }
|   |   expression-statement: {
|   |   |   binary op: {
|   |   |   |   identifer: `c`,
|   |   |   |   `<-`,
|   |   |   |   identifer: `b`,
|   |   |   },
|   |   `;`,
|   |   }
|   |   return statement: {
|   |   |   `return`,
|   |   |   identifer: `c`,
|   |   `;`,
|   |   }
|   |   `}`,
|   }
}
function declaration: {
|   `fn`,
|   `bar`,
|   `(`,
|   `)`,
|   block: {
|   |   `{`,
|   |   expression-statement: {
|   |   |   function call: {
|   |   |   |   identifer: `waste_effort`,
|   |   |   |   `(`,
|   |   |   |   `)`,
|   |   |   },
|   |   `;`,
|   |   }
|   |   return statement: {
|   |   |   `return`,
|   |   `;`,
|   |   }
|   |   `}`,
|   }
}
}
```
