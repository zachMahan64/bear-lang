# bear-lang (Work in Progress)
- A modern, C-like language inspired by C++ and Rust.
- Bear is statically-typed and compiled aiming for simplicity and clarity.
- Uses LLVM for the backend.

## Documentation
- [Grammar](docs/syntax.md)
- [HIR Design](docs/hir_design.md)
- [References](docs/references.md)

## Set up
#### Build
- See the [build docs](docs/build.md).
- You will need git, gcc or clang, CMake, and Ninja installed.
- The build also works using MinGW on Windows.
- Run these commands:
```
# for installing the LLVM distribution, do this first (which will take a while):
./scripts/llvm-install.sh              

# convient scripts:
./scripts/clean-all.sh [Release|Debug] # builds bearc and libbearc
./scripts/clean.sh     [Release|Debug] # builds bearc
./scripts/build-tests.sh               # build and run tests

# manual cmake build
cmake -B build -S .
cmake --build build
```
- Also, you will probably want to put `path/to/bearc/build/bearc/` on your path.
#### Run
```
bearc --help          # to see CLI usage
```
#### Preview
- Heres a preview of my AST pretty-printer as of 20260122:

```
// test/45.br
// some idiomatic bear code demo 

import "std/result.br";

fn main() {
    var my_cat = Animal..Cat(
                    AnimalInfo{
                        .name = String..from("Whiskers"), 
                        .age = 4},
                    3);
    var my_dog = Animal..Dog(
                    AnimalInfo{
                        .name = String..from("Sparky"), 
                        .age = 2},
                    3);
   pet_animal(my_cat); 
   pet_animal(my_dog);
   pet_animal(make_anacoda());
}

struct AnimalInfo {
    String name;
    u8 age;
}

deftype Decibel = i32;
deftype Inch = i32;

variant Animal {
    Dog(AnimalInfo info, Decibel barking_volume),
    Cat(AnimalInfo info, Inch whisker_length),
    Snake(AnimalInfo info),
    Lizard(AnimalInfo info),
}

fn make_anacoda() -> Animal..Snake {
    return Animal..Snake {
        .info = AnimalInfo {
            .name = String..from("Anacoda"),
            .age = 1,
        }
    };
}


fn pet_animal(Animal animal) { 
    var state = String..from("Reaching to pet a ");
    
    match (animal) {
        Animal..Cat(var info, var whis_len) => {
            state.append("cat!");
            state.append(" Its whiskers are ");
            state.append_string(String..from(whis_len));
            state.append(" inches long!");
        },
        Animal..Dog(var info, var bark_vol) => {
            state.append("dog!");
            state.append(" Its barks are ");
            state.append_string(String..from(bark_vol));
            state.append(" decibels loud!");
        }
        Animal..Snake | Animal..Lizard => {
            state.append(" repitilian! Oh hell nah!");
        }
    };
    use std;
    println(state);
}
```

```
file 'tests/45.br': {
import statement: {
|   `import`,
|   `"std/result.br"`,
},
function declaration: {
|   `fn`,
|   `main`,
|   `(`,
|   `)`,
|   block statement: {
|   |   `{`,
|   |   variable initialization: {
|   |   |   base type: {
|   |   |   |   `var`,
|   |   |   },
|   |   |   name: `my_cat`,
|   |   |   `=`,
|   |   |   function call: {
|   |   |   |   identifier: `Animal..Cat`,
|   |   |   |   `(`,
|   |   |   |   |   struct-init expression: {
|   |   |   |   |   name: `AnimalInfo`,
|   |   |   |   |   |   `{`,
|   |   |   |   |   |   member-init: {
|   |   |   |   |   |   |   `name`,
|   |   |   |   |   |   |   `=`,
|   |   |   |   |   |   |   function call: {
|   |   |   |   |   |   |   |   identifier: `String..from`,
|   |   |   |   |   |   |   |   `(`,
|   |   |   |   |   |   |   |   |   literal (string literal): `"Whiskers"`,
|   |   |   |   |   |   |   |   `)`,
|   |   |   |   |   |   |   },
|   |   |   |   |   |   },
|   |   |   |   |   |   member-init: {
|   |   |   |   |   |   |   `age`,
|   |   |   |   |   |   |   `=`,
|   |   |   |   |   |   |   literal (integer literal): `4`,
|   |   |   |   |   |   },
|   |   |   |   |   |   `}`,
|   |   |   |   |   },
|   |   |   |   |   `,`,
|   |   |   |   |   literal (integer literal): `3`,
|   |   |   |   `)`,
|   |   |   },
|   |   },
|   |   variable initialization: {
|   |   |   base type: {
|   |   |   |   `var`,
|   |   |   },
|   |   |   name: `my_dog`,
|   |   |   `=`,
|   |   |   function call: {
|   |   |   |   identifier: `Animal..Dog`,
|   |   |   |   `(`,
|   |   |   |   |   struct-init expression: {
|   |   |   |   |   name: `AnimalInfo`,
|   |   |   |   |   |   `{`,
|   |   |   |   |   |   member-init: {
|   |   |   |   |   |   |   `name`,
|   |   |   |   |   |   |   `=`,
|   |   |   |   |   |   |   function call: {
|   |   |   |   |   |   |   |   identifier: `String..from`,
|   |   |   |   |   |   |   |   `(`,
|   |   |   |   |   |   |   |   |   literal (string literal): `"Sparky"`,
|   |   |   |   |   |   |   |   `)`,
|   |   |   |   |   |   |   },
|   |   |   |   |   |   },
|   |   |   |   |   |   member-init: {
|   |   |   |   |   |   |   `age`,
|   |   |   |   |   |   |   `=`,
|   |   |   |   |   |   |   literal (integer literal): `2`,
|   |   |   |   |   |   },
|   |   |   |   |   |   `}`,
|   |   |   |   |   },
|   |   |   |   |   `,`,
|   |   |   |   |   literal (integer literal): `3`,
|   |   |   |   `)`,
|   |   |   },
|   |   },
|   |   expression-statement: {
|   |   |   function call: {
|   |   |   |   identifier: `pet_animal`,
|   |   |   |   `(`,
|   |   |   |   |   identifier: `my_cat`,
|   |   |   |   `)`,
|   |   |   },
|   |   |   `;`,
|   |   },
|   |   expression-statement: {
|   |   |   function call: {
|   |   |   |   identifier: `pet_animal`,
|   |   |   |   `(`,
|   |   |   |   |   identifier: `my_dog`,
|   |   |   |   `)`,
|   |   |   },
|   |   |   `;`,
|   |   },
|   |   expression-statement: {
|   |   |   function call: {
|   |   |   |   identifier: `pet_animal`,
|   |   |   |   `(`,
|   |   |   |   |   function call: {
|   |   |   |   |   |   identifier: `make_anacoda`,
|   |   |   |   |   |   `(`,
|   |   |   |   |   |   `)`,
|   |   |   |   |   },
|   |   |   |   `)`,
|   |   |   },
|   |   |   `;`,
|   |   },
|   |   `}`,
|   },
},
struct declaration: {
|   `struct`,
|   name: `AnimalInfo`,
|   fields: {
|   |   variable declaration: {
|   |   |   base type: {
|   |   |   |   `String`,
|   |   |   },
|   |   |   name: `name`,
|   |   },
|   |   variable declaration: {
|   |   |   base type: {
|   |   |   |   `u8`,
|   |   |   },
|   |   |   name: `age`,
|   |   },
|   },
},
deftype alias: {
|   `deftype`,
|   name: `Decibel`,
|   `=`,
|   type-expression: {
|   |   base type: {
|   |   |   `i32`,
|   |   },
|   },
},
deftype alias: {
|   `deftype`,
|   name: `Inch`,
|   `=`,
|   type-expression: {
|   |   base type: {
|   |   |   `i32`,
|   |   },
|   },
},
variant declaration: {
|   `variant`,
|   name: `Animal`,
|   fields: {
|   |   variant field: {
|   |   |   name: `Dog`,
|   |   |   `(`,
|   |   |   parameter: {
|   |   |   |   base type: {
|   |   |   |   |   `AnimalInfo`,
|   |   |   |   },
|   |   |   |   name: `info`,
|   |   |   },
|   |   |   parameter: {
|   |   |   |   base type: {
|   |   |   |   |   `Decibel`,
|   |   |   |   },
|   |   |   |   name: `barking_volume`,
|   |   |   },
|   |   |   `)`,
|   |   },
|   |   variant field: {
|   |   |   name: `Cat`,
|   |   |   `(`,
|   |   |   parameter: {
|   |   |   |   base type: {
|   |   |   |   |   `AnimalInfo`,
|   |   |   |   },
|   |   |   |   name: `info`,
|   |   |   },
|   |   |   parameter: {
|   |   |   |   base type: {
|   |   |   |   |   `Inch`,
|   |   |   |   },
|   |   |   |   name: `whisker_length`,
|   |   |   },
|   |   |   `)`,
|   |   },
|   |   variant field: {
|   |   |   name: `Snake`,
|   |   |   `(`,
|   |   |   parameter: {
|   |   |   |   base type: {
|   |   |   |   |   `AnimalInfo`,
|   |   |   |   },
|   |   |   |   name: `info`,
|   |   |   },
|   |   |   `)`,
|   |   },
|   |   variant field: {
|   |   |   name: `Lizard`,
|   |   |   `(`,
|   |   |   parameter: {
|   |   |   |   base type: {
|   |   |   |   |   `AnimalInfo`,
|   |   |   |   },
|   |   |   |   name: `info`,
|   |   |   },
|   |   |   `)`,
|   |   },
|   },
},
function declaration: {
|   `fn`,
|   `make_anacoda`,
|   `(`,
|   `)`,
|   `->`,
|   base type: {
|   |   `Animal..Snake`,
|   },
|   block statement: {
|   |   `{`,
|   |   return statement: {
|   |   |   `return`,
|   |   |   struct-init expression: {
|   |   |   name: `Animal..Snake`,
|   |   |   |   `{`,
|   |   |   |   member-init: {
|   |   |   |   |   `info`,
|   |   |   |   |   `=`,
|   |   |   |   |   struct-init expression: {
|   |   |   |   |   name: `AnimalInfo`,
|   |   |   |   |   |   `{`,
|   |   |   |   |   |   member-init: {
|   |   |   |   |   |   |   `name`,
|   |   |   |   |   |   |   `=`,
|   |   |   |   |   |   |   function call: {
|   |   |   |   |   |   |   |   identifier: `String..from`,
|   |   |   |   |   |   |   |   `(`,
|   |   |   |   |   |   |   |   |   literal (string literal): `"Anacoda"`,
|   |   |   |   |   |   |   |   `)`,
|   |   |   |   |   |   |   },
|   |   |   |   |   |   },
|   |   |   |   |   |   member-init: {
|   |   |   |   |   |   |   `age`,
|   |   |   |   |   |   |   `=`,
|   |   |   |   |   |   |   literal (integer literal): `1`,
|   |   |   |   |   |   },
|   |   |   |   |   |   `}`,
|   |   |   |   |   },
|   |   |   |   },
|   |   |   |   `}`,
|   |   |   },
|   |   |   `;`,
|   |   },
|   |   `}`,
|   },
},
function declaration: {
|   `fn`,
|   `pet_animal`,
|   `(`,
|   parameter: {
|   |   base type: {
|   |   |   `Animal`,
|   |   },
|   |   name: `animal`,
|   },
|   `)`,
|   block statement: {
|   |   `{`,
|   |   variable initialization: {
|   |   |   base type: {
|   |   |   |   `var`,
|   |   |   },
|   |   |   name: `state`,
|   |   |   `=`,
|   |   |   function call: {
|   |   |   |   identifier: `String..from`,
|   |   |   |   `(`,
|   |   |   |   |   literal (string literal): `"Reaching to pet a "`,
|   |   |   |   `)`,
|   |   |   },
|   |   },
|   |   expression-statement: {
|   |   |   match-expression: {
|   |   |   `match`,
|   |   |   |   `(`,
|   |   |   |   identifier: `animal`,
|   |   |   |   `)`,
|   |   |   |    match-branch: {
|   |   |   |   |   variant decomposition: {
|   |   |   |   |   |   name: `Animal..Cat`,
|   |   |   |   |   |   `(`,
|   |   |   |   |   |   parameter: {
|   |   |   |   |   |   |   base type: {
|   |   |   |   |   |   |   |   `var`,
|   |   |   |   |   |   |   },
|   |   |   |   |   |   |   name: `info`,
|   |   |   |   |   |   },
|   |   |   |   |   |   parameter: {
|   |   |   |   |   |   |   base type: {
|   |   |   |   |   |   |   |   `var`,
|   |   |   |   |   |   |   },
|   |   |   |   |   |   |   name: `whis_len`,
|   |   |   |   |   |   },
|   |   |   |   |   |   `)`,
|   |   |   |   |   },
|   |   |   |   |   `=>`,
|   |   |   |   |   block-expression: {
|   |   |   |   |   |   `{`,
|   |   |   |   |   |   expression-statement: {
|   |   |   |   |   |   |   binary-expr: {
|   |   |   |   |   |   |   |   identifier: `state`,
|   |   |   |   |   |   |   |   `.`,
|   |   |   |   |   |   |   |   function call: {
|   |   |   |   |   |   |   |   |   identifier: `append`,
|   |   |   |   |   |   |   |   |   `(`,
|   |   |   |   |   |   |   |   |   |   literal (string literal): `"cat!"`,
|   |   |   |   |   |   |   |   |   `)`,
|   |   |   |   |   |   |   |   },
|   |   |   |   |   |   |   },
|   |   |   |   |   |   |   `;`,
|   |   |   |   |   |   },
|   |   |   |   |   |   expression-statement: {
|   |   |   |   |   |   |   binary-expr: {
|   |   |   |   |   |   |   |   identifier: `state`,
|   |   |   |   |   |   |   |   `.`,
|   |   |   |   |   |   |   |   function call: {
|   |   |   |   |   |   |   |   |   identifier: `append`,
|   |   |   |   |   |   |   |   |   `(`,
|   |   |   |   |   |   |   |   |   |   literal (string literal): `" Its whiskers are "`,
|   |   |   |   |   |   |   |   |   `)`,
|   |   |   |   |   |   |   |   },
|   |   |   |   |   |   |   },
|   |   |   |   |   |   |   `;`,
|   |   |   |   |   |   },
|   |   |   |   |   |   expression-statement: {
|   |   |   |   |   |   |   binary-expr: {
|   |   |   |   |   |   |   |   identifier: `state`,
|   |   |   |   |   |   |   |   `.`,
|   |   |   |   |   |   |   |   function call: {
|   |   |   |   |   |   |   |   |   identifier: `append_string`,
|   |   |   |   |   |   |   |   |   `(`,
|   |   |   |   |   |   |   |   |   |   function call: {
|   |   |   |   |   |   |   |   |   |   |   identifier: `String..from`,
|   |   |   |   |   |   |   |   |   |   |   `(`,
|   |   |   |   |   |   |   |   |   |   |   |   identifier: `whis_len`,
|   |   |   |   |   |   |   |   |   |   |   `)`,
|   |   |   |   |   |   |   |   |   |   },
|   |   |   |   |   |   |   |   |   `)`,
|   |   |   |   |   |   |   |   },
|   |   |   |   |   |   |   },
|   |   |   |   |   |   |   `;`,
|   |   |   |   |   |   },
|   |   |   |   |   |   expression-statement: {
|   |   |   |   |   |   |   binary-expr: {
|   |   |   |   |   |   |   |   identifier: `state`,
|   |   |   |   |   |   |   |   `.`,
|   |   |   |   |   |   |   |   function call: {
|   |   |   |   |   |   |   |   |   identifier: `append`,
|   |   |   |   |   |   |   |   |   `(`,
|   |   |   |   |   |   |   |   |   |   literal (string literal): `" inches long!"`,
|   |   |   |   |   |   |   |   |   `)`,
|   |   |   |   |   |   |   |   },
|   |   |   |   |   |   |   },
|   |   |   |   |   |   |   `;`,
|   |   |   |   |   |   },
|   |   |   |   |   |   `}`,
|   |   |   |   |   },
|   |   |   |   },
|   |   |   |    match-branch: {
|   |   |   |   |   variant decomposition: {
|   |   |   |   |   |   name: `Animal..Dog`,
|   |   |   |   |   |   `(`,
|   |   |   |   |   |   parameter: {
|   |   |   |   |   |   |   base type: {
|   |   |   |   |   |   |   |   `var`,
|   |   |   |   |   |   |   },
|   |   |   |   |   |   |   name: `info`,
|   |   |   |   |   |   },
|   |   |   |   |   |   parameter: {
|   |   |   |   |   |   |   base type: {
|   |   |   |   |   |   |   |   `var`,
|   |   |   |   |   |   |   },
|   |   |   |   |   |   |   name: `bark_vol`,
|   |   |   |   |   |   },
|   |   |   |   |   |   `)`,
|   |   |   |   |   },
|   |   |   |   |   `=>`,
|   |   |   |   |   block-expression: {
|   |   |   |   |   |   `{`,
|   |   |   |   |   |   expression-statement: {
|   |   |   |   |   |   |   binary-expr: {
|   |   |   |   |   |   |   |   identifier: `state`,
|   |   |   |   |   |   |   |   `.`,
|   |   |   |   |   |   |   |   function call: {
|   |   |   |   |   |   |   |   |   identifier: `append`,
|   |   |   |   |   |   |   |   |   `(`,
|   |   |   |   |   |   |   |   |   |   literal (string literal): `"dog!"`,
|   |   |   |   |   |   |   |   |   `)`,
|   |   |   |   |   |   |   |   },
|   |   |   |   |   |   |   },
|   |   |   |   |   |   |   `;`,
|   |   |   |   |   |   },
|   |   |   |   |   |   expression-statement: {
|   |   |   |   |   |   |   binary-expr: {
|   |   |   |   |   |   |   |   identifier: `state`,
|   |   |   |   |   |   |   |   `.`,
|   |   |   |   |   |   |   |   function call: {
|   |   |   |   |   |   |   |   |   identifier: `append`,
|   |   |   |   |   |   |   |   |   `(`,
|   |   |   |   |   |   |   |   |   |   literal (string literal): `" Its barks are "`,
|   |   |   |   |   |   |   |   |   `)`,
|   |   |   |   |   |   |   |   },
|   |   |   |   |   |   |   },
|   |   |   |   |   |   |   `;`,
|   |   |   |   |   |   },
|   |   |   |   |   |   expression-statement: {
|   |   |   |   |   |   |   binary-expr: {
|   |   |   |   |   |   |   |   identifier: `state`,
|   |   |   |   |   |   |   |   `.`,
|   |   |   |   |   |   |   |   function call: {
|   |   |   |   |   |   |   |   |   identifier: `append_string`,
|   |   |   |   |   |   |   |   |   `(`,
|   |   |   |   |   |   |   |   |   |   function call: {
|   |   |   |   |   |   |   |   |   |   |   identifier: `String..from`,
|   |   |   |   |   |   |   |   |   |   |   `(`,
|   |   |   |   |   |   |   |   |   |   |   |   identifier: `bark_vol`,
|   |   |   |   |   |   |   |   |   |   |   `)`,
|   |   |   |   |   |   |   |   |   |   },
|   |   |   |   |   |   |   |   |   `)`,
|   |   |   |   |   |   |   |   },
|   |   |   |   |   |   |   },
|   |   |   |   |   |   |   `;`,
|   |   |   |   |   |   },
|   |   |   |   |   |   expression-statement: {
|   |   |   |   |   |   |   binary-expr: {
|   |   |   |   |   |   |   |   identifier: `state`,
|   |   |   |   |   |   |   |   `.`,
|   |   |   |   |   |   |   |   function call: {
|   |   |   |   |   |   |   |   |   identifier: `append`,
|   |   |   |   |   |   |   |   |   `(`,
|   |   |   |   |   |   |   |   |   |   literal (string literal): `" decibels loud!"`,
|   |   |   |   |   |   |   |   |   `)`,
|   |   |   |   |   |   |   |   },
|   |   |   |   |   |   |   },
|   |   |   |   |   |   |   `;`,
|   |   |   |   |   |   },
|   |   |   |   |   |   `}`,
|   |   |   |   |   },
|   |   |   |   },
|   |   |   |    match-branch: {
|   |   |   |   |   identifier: `Animal..Snake`,
|   |   |   |   |   `|`,
|   |   |   |   |   identifier: `Animal..Lizard`,
|   |   |   |   |   `=>`,
|   |   |   |   |   block-expression: {
|   |   |   |   |   |   `{`,
|   |   |   |   |   |   expression-statement: {
|   |   |   |   |   |   |   binary-expr: {
|   |   |   |   |   |   |   |   identifier: `state`,
|   |   |   |   |   |   |   |   `.`,
|   |   |   |   |   |   |   |   function call: {
|   |   |   |   |   |   |   |   |   identifier: `append`,
|   |   |   |   |   |   |   |   |   `(`,
|   |   |   |   |   |   |   |   |   |   literal (string literal): `" repitilian! Oh hell nah!"`,
|   |   |   |   |   |   |   |   |   `)`,
|   |   |   |   |   |   |   |   },
|   |   |   |   |   |   |   },
|   |   |   |   |   |   |   `;`,
|   |   |   |   |   |   },
|   |   |   |   |   |   `}`,
|   |   |   |   |   },
|   |   |   |   },
|   |   |   },
|   |   |   `;`,
|   |   },
|   |   use statement {
|   |   |   identifier: `std`,
|   |   },
|   |   expression-statement: {
|   |   |   function call: {
|   |   |   |   identifier: `println`,
|   |   |   |   `(`,
|   |   |   |   |   identifier: `state`,
|   |   |   |   `)`,
|   |   |   },
|   |   |   `;`,
|   |   },
|   |   `}`,
|   },
},
},
```
