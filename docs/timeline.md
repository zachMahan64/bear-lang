### Dev stages/checkpoints

#### Lexer
- [x] Working lexer for all tokens

## Imperative and Arithmetic 

#### Declarations and operators/precedence
- [ ] `i32`, etc (integral)
- [ ] `f32`, `f64` (floating)
- [ ] `char`
- [ ] `bool`
- [ ] Arithmetic and boolean operators
- [ ] `=` and `<-` assignment operators

## Procedural

#### I) Control Flow (branches, loops) and basic console-out stream
- `if`, `else`
- `while`, `for`, `for ... in ...`
- `cout <<- ...` or prints

#### II) Free-function calls and scope resolution (modules)

#### III) Memory semantics + more advanced ownership-based diagnostics?
- [ ] `box` and `bag`
- [ ] `str` and string literal handling (just store immutably in static memory)

#### IV) Arrays
- [ ] Fixed-size stack arrays
- [ ] slices
- [ ] `box` and `bag` arrays

#### V) Compile Time Constants
- [ ] `compt` -> support for integral and floating constants
- [ ] `enum` -> type-checked scoped constants

## Data-Based

#### I) Basic structs and methods, struct scope resolution
- [ ] `mt`, `fn` (static methods), `dt` -> dtor

#### II) More struct features
- [ ] `static` and `hid` data members
- [ ] `hid` functions/methods


#### III) Advanced struct features
- [ ] `template` -> basic macro-like templates

## Real-worldifications
#### I) 
- [ ] STL/stdlib
#### II) Bindable to C
#### III) LSP
- Use Rust, link to libbearc
- [ ] VSCode Compatible 
- [ ] NeoVim Compatible 
- Reuse Compiler Tokens and Diagnostic Emission
