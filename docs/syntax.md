### BearLang Syntax
- **Work In Progress:** this document will be the main syntax reference before moving to a more rigorous system of seperate documents for each language feature.
- BearLang is a C-Like language inspired by elements of Rust, and C++.
#### Project structure:
- Bear uses a module-based structure. At first, modules will also have to be compiled before I can add static library + metadata files.
- This is the basics of the system:
```
// in main.br 
import my_mod // a module

fn main(str[]& args) -> i32 {
    my_mod..do_thing();
    return 0;
}
```
- From the root dir, the compiler will search for ./my_mod then try ./my_mod/my_mod 
- Whichever one it finds is the root of that module
```
// in ./my_mod
space my_mod; // the rest of the file will be apart of the "my_mod" namespace

fn do_thing() {
    cout <<- "I'm doing a thing\n";
}

```
#### Built-in types:
- `i8, u8, ..., i64, u64` supported
- `f32` single and `f64` double precision
- char (unicode 32-bit)
- str (utf-8 string)
- bool
#### Type-related features (will come later)
- type-deduction: `auto x = func_returning_i32(); // x will be i32`
- compt: `compt i32 x = some_compt_func();`
#### Moves / Copies 
- All objects are copiable and moveable, unless marked with `NoCopy`, `NoMove`
- `=` for copies, `<-` for moves during assignment 
- All copies and moves are bitwise
- Moved-from objects will never have their dtor's called
- You cannot move fields out of objects
#### Variables
```
i32 x = 0; // assignment
i32 y <- x; // move assignment
i32 z = crunch_numbers();
BigStruct big_thing = get_big_thing();
ResrcManager manager1 = ResrcManager::new();
ResrcManager manager2 <- manager1; // transfer ownership through move
```
#### Arrays
```
i32[10] my_arr;
i32[10]& my_slice; // slice holds ptr + len
```
#### Templated Types (will come later)
    - `MyTemplatedType<param1,param2,param3>`
    - `box::MyTemplatedType<param1,param2,param3>`
