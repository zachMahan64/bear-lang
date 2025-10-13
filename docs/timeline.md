### Goals and Checkpoints

## Warm-Up
#### Lexer
- [x] Working Lexer for all tokens
#### Reading
- [ ] Read relevant chapters of *Crafting Interpreters*

## Imperative and Arithmetic 

#### A Game of Declarations and Operator Precedence
- [ ] `int`, `long` (integral)
- [ ] `flt`, `doub` (floating)
- [ ] `char`
- [ ] `bool`
- [ ] Arithmetic and boolean operators
- [ ] `=` and `<-` assignment operators
- [ ] AST 
- [ ] Codegen
- [ ] Interpreter Runtime

## Procedural

#### I) Free-Function Calls and Scope Resolution (spaces)
- [ ] AST 
- [ ] Codegen
- [ ] Interpreter Runtime

#### II) Control Flow (branches, loops) and Basic Console-Out Stream
- `if`, `elif`, `else`
- `while`, `for`, `for ... in ...`
- `cout <<- ...`

- [ ] AST 
- [ ] Codegen
- [ ] Interpreter Runtime

#### Helpful Detour -> ByteCode Decompiler and Step-Through Debugger

#### III) Memory Semantics & More Advanced Compiler Errors
- [ ] `box`, `bag`, and `ref`
- [ ] `str` and string literal handling (just store immutably in static memory)
- [ ] AST 
- [ ] Codegen
- [ ] Interpreter Runtime (exceptions?)

#### IV) Fixed-Size Arrays
- [ ] Stack Arrays 
- [ ] `box` and `bag` arrays
- [ ] AST 
- [ ] Codegen
- [ ] Interpreter Runtime

#### V) Compile Time Constants
- [ ] `comp` -> support for integral and floating constants
- [ ] `enum` -> type-checked scoped constants
- [ ] AST 
- [ ] Codegen

## Data-Based

#### I) Basic Structs and Methods; Struct Scope Resolution
- [ ] `mt`, `fn` (static methods), `ct`/`dt` -> ctor/dtor
- [ ] AST 
- [ ] Codegen
- [ ] Runtime Implementation

#### II) More Struct Features
- [ ] `static` and `hidden` data members
- [ ] `hidden` functions/methods
- [ ] AST 
- [ ] Codegen
- [ ] Runtime Implementation


#### III) Advanced Struct Features
- [ ] `template` -> templates and type-glue semantics
- [ ] AST 
- [ ] Codegen
- [ ] Runtime Implementation

## Real-Worldifications

#### I) Bindable to C
- [ ] Outline
- [ ] AST 
- [ ] Codegen
- [ ] Runtime Implementation

#### II) Better Compiler Error Checking, Warnings, and Hints
- [ ] AST 
- [ ] Codegen

#### III) LSP
- *Use TypeScript, Go, C++, or Rust? -> must be portable*
- [ ] VSCode Compatible 
- [ ] NeoVim Compatible
- [ ] AST 
- [ ] Codegen

#### IV) Bindable to Python?

- [ ] Outline
- [ ] AST 
- [ ] Codegen
- [ ] Runtime Implementation

## Next
- [ ] Make a binary-compiled backend?
- [ ] Bring other people onboard
