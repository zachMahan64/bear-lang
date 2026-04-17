<div align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://github.com/bear-language/bear-vscode/blob/main/images/bear-logo-text-dark.png?raw=true">
    <source media="(prefers-color-scheme: light)" srcset="https://github.com/bear-language/bear-vscode/blob/main/images/bear-logo-text-light.png?raw=true">
    <img alt="The Bear Programming Language"
         src="https://raw.githubusercontent.com/zachMahan64/bear-vscode/refs/heads/main/images/bear-logo.png"
         width="50%">
  </picture>

</div>

-------------
- A modern, powerful, C-like language with simple syntax.
- Bear is statically-typed and compiled using LLVM as the backend.
- Codegen is still a work-in-progress.

## Documentation
- [Grammar](docs/syntax.md)
- [References](docs/references.md)

## Set up
#### Build
- See the [build docs](docs/build.md).
- You will need git, gcc or clang, CMake, and Ninja installed.
- The build also works using MinGW on Windows.
- Run these commands:
```
# for installing the LLVM distribution, do this first (which will take a while):
./scripts/llvm-init.sh              

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
#### Tooling 
- [A Neovim plugin is availible](https://github.com/bear-language/bear.nvim), which easily can be used in Vim as well.
- [A VSCode extension is availible](https://github.com/bear-language/bear-vscode), which includes syntax highlighting. Right now, local install is required, directions are in the linked repo.
- More to come in the future

#### Previews
- Bear is comfortably turing complete at compile-time, with many composable reflection and type-inference features
``` bear
// tests/hir/59.br

// let's show off some reflection, compile-time execution, 
// and static duck-typing

mod demo; 

compt fn square_val(var a) => a * a

compt fn is_integer(var i) => {
    @same_type(typeof i, i64) || @same_type(typeof i, u64) ||
    @same_type(typeof i, i32) || @same_type(typeof i, u32) ||
    @same_type(typeof i, i16) || @same_type(typeof i, u16) ||
    @same_type(typeof i, i8) || @same_type(typeof i, u8) 
}

struct IntWrapper {

    deftype ValType = i32;

    hid ValType stored_val;

    fn new(ValType val) => Self{.stored_val = val}

    mt copy() => Self{.stored_val = self.stored_val}

    mt val() -> ValType => self.stored_val

    compt mt add_with(var a) =>
        Self..new(self.val() + a) if @same_type(decay typeof a, Self)
            else Self..new(self.val() + a as typeof stored_val)

    compt mt square() => Self..new(square_val(self.val()))

    // this can take in any list thru compile-time reflection
    compt mt val_in_list(var list) -> bool => { 
        self.val_in_list_helper(list, list.len - 1)
        // self.val_in_list_helper(list, list.len) if @defined(list.len) 
        //    else false // not a list storing value type
    }
    
    // recursive helper
    hid compt mt val_in_list_helper(var list, var curr) -> bool => { 
        false if curr == 0 
            else (list[curr] == self.val()) || self.val_in_list_helper(list, curr - 1)
    }

}

//                  tests
// -------------------------------------------------
// these are all evaluatable at compile-time
// note: @static_assert is a compile-time expression 
// that returns a bool, so that's why we're writing
// our tests like this:

deftype Int = IntWrapper; // we can define a type alias like this

var _0 = @static_assert Int..new(12).val() == 12; 

var _1 = @static_assert @same_type(Int..ValType, i32); // ensure our type alias is working as intended

compt Int my_int = Int..new(42); // we can check equalities of structs at compile-time too

var _2 = @static_assert my_int.add_with(1000) == Int..new(1000 + 42);

use Int..ValType; // use Int aka IntWrapper's ValType in current scope

var _3 = @static_assert @same_type(ValType, i32);

var _4 = @static_assert @same_type(decay &mut ValType, i32); // decay discards qualifiers, so this should hold!

var _5 = @static_assert Int..new(12).square().square().val() == 12 * 12 * 12 * 12;

compt Int my_int1 = Int..new(10);

var _6 = @static_assert my_int1.val_in_list([1, 2, 3, 4, 7, 10, 8, 9, 1000]);

var _7 = @static_assert Int..new(12).copy() == Int..new(12); // copy called as a method

var _8 = @static_assert Int..copy(Int..new(12)) == Int{.stored_val = 12}; // methods can also take Self as a 
                                                                          // first regular argument

// some mistakes to demo diagnostics:

var _9 = @static_assert my_int1.val_in_list([1, 2, 3, 4, 7, 11, 8, 9, 1000]); // oops

compt ValType v = my_int1.val(0) == Int..new(); // oops, too many and too few arguments

compt i32 v1 = Int..new(42); // oops, not convertible here
```
- Diagnostics output from the above test source code (less pretty here than normal tty output since there's no bold/color here)
```
error: static assertion failed 
 --> /Users/zachmahan/dev/bear-lang/tests/hir/59.br:86:25 
      | 
  86  | var _9 = @static_assert my_int1.val_in_list([1, 2, 3, 4, 7, 11, 8, 9, 1000]); // oops
      |                         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ condition is false
      | 

error: function `val` expected 0 arguments but got 1 
 --> /Users/zachmahan/dev/bear-lang/tests/hir/59.br:88:31 
      | 
  88  | compt ValType v = my_int1.val(0) == Int..new(); // oops, too many and too few arguments
      |                               ^ 
note: `val` declared here 
      | 
  27  |     mt val() -> ValType => self.stored_val
      |        ^^^ 

error: function `new` expected 1 arguments but got 0 
 --> /Users/zachmahan/dev/bear-lang/tests/hir/59.br:88:46 
      | 
  88  | compt ValType v = my_int1.val(0) == Int..new(); // oops, too many and too few arguments
      |                                              ^ 
note: `new` declared here 
      | 
  23  |     fn new(ValType val) => Self{.stored_val = val}
      |        ^^^ 

error: cannot convert value of type `IntWrapper` to `i32` 
 --> /Users/zachmahan/dev/bear-lang/tests/hir/59.br:90:16 
      | 
  90  | compt i32 v1 = Int..new(42); // oops, not convertible here
      |                ^^^^^^^^^^^^ 

4 errors generated.
2 notes generated.
compilation terminated: '/Users/zachmahan/dev/bear-lang/tests/hir/59.br'
```

- Heres a preview of run-time code with the AST pretty-printed (again less pretty here than tty-output since there's no bold/color):

``` bear
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
