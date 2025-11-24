# Goals and Checkpoints

## Warm-Up
#### Lexer
- [x] Working Lexer for all tokens


## Reading, to be done throughout the project as a guide
- [ ] Read relevant chapters of *Crafting Interpreters*
- [ ] Read *Engineering a Compiler*

## Things to Keep In Mind
#### Across this Timeline:
- **Every "Parser/Semantic Analysis" phase includes:**
    - Name resolution
    - Type inference / checking
    - Borrow and ownership checks (later)
    - Const-eval and fold 
- **Every Codegen Phase:**
    - Update the following to support the added feature: 
        - Linear IR
        - SSA

#### Approach to Diagnostics
- **This must be considered throughout developement of the compiler for future LSP compatibility.**
    - Unified diagnostic emitter with span + severity
    - Error, warning, note/hint hierarchy
    - Line/Column tracking

#### Setting up for Future Codegen
- [ ] Implement lowering from AST to the IR 

## Imperative and Arithmetic 

#### Declarations and Operator Precedence
- [ ] `i32`, etc (integral)
- [ ] `f32`, `f64` (floating)
- [ ] `char`
- [ ] `bool`
- [ ] Arithmetic and boolean operators
- [ ] `=` and `<-` assignment operators

- [ ] Parser/Semantic Analysis
- [ ] AST 
- [ ] Codegen

## Procedural

#### I) Control Flow (branches, loops) and Basic Console-Out Stream
- `if`, `elif`, `else`
- `while`, `for`, `for ... in ...`
- `cout <<- ...`

- [ ] Parser/Semantic Analysis
- [ ] AST 
- [ ] Codegen

#### II) Free-Function Calls and Scope Resolution (modules)
- [ ] AST 
- [ ] Codegen

#### III) Memory Semantics & More Advanced Compiler Errors
- [ ] `box` and `bag`
- [ ] `str` and string literal handling (just store immutably in static memory)
- [ ] Parser/Semantic Analysis
- [ ] AST 
- [ ] Codegen

#### IV) Fixed-Size Arrays
- [ ] Stack Arrays 
- [ ] `box` and `bag` arrays

- [ ] Parser/Semantic Analysis
- [ ] AST 
- [ ] Codegen

#### V) Compile Time Constants
- [ ] `compt` -> support for integral and floating constants
- [ ] `enum` -> type-checked scoped constants

- [ ] Parser/Semantic Analysis
- [ ] AST 
- [ ] Codegen

## Data-Based

#### I) Basic Structs and Methods; Struct Scope Resolution
- [ ] `mt`, `fn` (static methods), `dt` -> dtor

- [ ] Parser/Semantic Analysis
- [ ] AST 
- [ ] Codegen
- [ ] Runtime Implementation

#### II) More Struct Features
- [ ] `static` and `hid` data members
- [ ] `hid` functions/methods

- [ ] Parser/Semantic Analysis
- [ ] AST 
- [ ] Codegen
- [ ] Runtime Implementation


#### III) Advanced Struct Features
- [ ] `template` -> basic macro-like templates

- [ ] Parser/Semantic Analysis
- [ ] AST 
- [ ] Codegen
- [ ] Runtime Implementation

## Real-Worldifications

#### I) Bindable to C
- [ ] Outline

- [ ] Parser/Semantic Analysis
- [ ] AST 
- [ ] Codegen
- [ ] Runtime Implementation

#### II) Better Compiler Error Checking, Warnings, and Hints
- [ ] Parser/Semantic Analysis
- [ ] AST 
- [ ] Codegen

#### III) LSP
- *Use TypeScript, Go, C++, or Rust? -> must be portable*
- [ ] VSCode Compatible 
- [ ] NeoVim Compatible 
- Reuse Compiler Tokens and Diagnostic Emission
