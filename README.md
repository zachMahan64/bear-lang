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
- Heres a preview of my AST pretty-printer as of 20251218:
```
// tests/9.br

73.0 as u8 + std..something(arg1++, --arg2);

a..b.c + std..some_call(5);

hi[a];
{call();}

i32 a;

a = 42;

i32 b = a;
```
```
file 'tests/9.br': {
expression-statement: {
|   binary op: {
|   |   binary op: {
|   |   |   literal (doub_lit): `73.0`,
|   |   |   `as`,
|   |   |   identifer: `u8`,
|   |   },
|   |   `+`,
|   |   function call: {
|   |   |   identifer: `std..something`,
|   |   |   `(`,
|   |   |   |   post-unary: {
|   |   |   |   |   identifer: `arg1`,
|   |   |   |   |   `++`,
|   |   |   |   },
|   |   |   |   `,`,
|   |   |   |   pre-unary: {
|   |   |   |   |   `--`,
|   |   |   |   |   identifer: `arg2`,
|   |   |   |   },
|   |   |   `)`,
|   |   },
|   },
`;`,
}
expression-statement: {
|   binary op: {
|   |   binary op: {
|   |   |   identifer: `a..b`,
|   |   |   `.`,
|   |   |   identifer: `c`,
|   |   },
|   |   `+`,
|   |   function call: {
|   |   |   identifer: `std..some_call`,
|   |   |   `(`,
|   |   |   |   literal (int_lit): `5`,
|   |   |   `)`,
|   |   },
|   },
`;`,
}
expression-statement: {
|   subscript: {
|   |   identifer: `hi`,
|   |   `[`,
|   |   |   identifer: `a`,
|   |   `]`,
|   },
`;`,
}
block: {
|   `{`,
|   expression-statement: {
|   |   function call: {
|   |   |   identifer: `call`,
|   |   |   `(`,
|   |   |   `)`,
|   |   },
|   `;`,
|   }
|   `}`,
}
variable declaration: {
|   type: `i32`,
|   name: `a`,
`;`,
}
expression-statement: {
|   binary op: {
|   |   identifer: `a`,
|   |   `=`,
|   |   literal (int_lit): `42`,
|   },
`;`,
}
variable initialization: {
|   type: `i32`,
|   name: `b`,
|   `=`,
|   identifer: `a`,
`;`,
}
}
```
