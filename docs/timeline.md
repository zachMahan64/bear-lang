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
        - SSA?
        - Emission of ByteCode

#### Approach to Diagnostics
- **This must be considered throughout developement of the compiler for future LSP compatibility.**
- [ ] Unified diagnostic emitter with span + severity
- [ ] Error, Warning, Note hierarchy
- [ ] Integration with LSP later


## Setting up for Future Codegen and Designing the ByteCode Interpreter
#### Intermediate Representation
- [ ] Define stable linear IR
- [ ] Optional SSA
- [ ] Implement lowering from AST to the IR 
#### Defining the Requirements of the VM
- [ ] Define VM opcodes and instruction format (make sure virtual-registers are the right choice)
#### Moving from IR to ByteCode
- [ ] Design an efficient pipeline for IR (unlimited virtual-register based, like LLVM) to BVM ByteCode

## Imperative and Arithmetic 

#### A Game of Declarations and Operator Precedence
- [ ] `int`, `long` (integral)
- [ ] `flt`, `doub` (floating)
- [ ] `char`
- [ ] `bool`
- [ ] Arithmetic and boolean operators
- [ ] `=` and `<-` assignment operators

- [ ] Parser/Semantic Analysis
- [ ] AST 
- [ ] Codegen
- [ ] Interpreter Runtime

## Procedural

#### I) Control Flow (branches, loops) and Basic Console-Out Stream
- `if`, `elif`, `else`
- `while`, `for`, `for ... in ...`
- `cout <<- ...`

- [ ] Parser/Semantic Analysis
- [ ] AST 
- [ ] Codegen
- [ ] Interpreter Runtime

#### II) Free-Function Calls and Scope Resolution (spaces)
- [ ] AST 
- [ ] Codegen
- [ ] Interpreter Runtime

#### III) Memory Semantics & More Advanced Compiler Errors
- [ ] `box`, `bag`, and `ref`
- [ ] `str` and string literal handling (just store immutably in static memory)
- [ ] Parser/Semantic Analysis
- [ ] AST 
- [ ] Codegen
- [ ] Interpreter Runtime (exceptions?)

#### IV) Fixed-Size Arrays
- [ ] Stack Arrays 
- [ ] `box` and `bag` arrays

- [ ] Parser/Semantic Analysis
- [ ] AST 
- [ ] Codegen
- [ ] Interpreter Runtime

#### V) Compile Time Constants
- [ ] `comp` -> support for integral and floating constants
- [ ] `enum` -> type-checked scoped constants

- [ ] Parser/Semantic Analysis
- [ ] AST 
- [ ] Codegen

## Helpful Sanity Detour
- [ ] ByteCode Decompiler
- [ ] Step-Through Debugger

## Data-Based

#### I) Basic Structs and Methods; Struct Scope Resolution
- [ ] `mt`, `fn` (static methods), `ct`/`dt` -> ctor/dtor

- [ ] Parser/Semantic Analysis
- [ ] AST 
- [ ] Codegen
- [ ] Runtime Implementation

#### II) More Struct Features
- [ ] `static` and `hidden` data members
- [ ] `hidden` functions/methods

- [ ] Parser/Semantic Analysis
- [ ] AST 
- [ ] Codegen
- [ ] Runtime Implementation


#### III) Advanced Struct Features
- [ ] `template` -> basic macro-like templates and type-glue semantics

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

#### IV) Bindable to Python?
- [ ] Outline

- [ ] Parser/Semantic Analysis
- [ ] AST 
- [ ] Codegen
- [ ] Runtime Implementation

## Next
- [ ] Make a native binary-compiled backend?
- [ ] Bring other people onboard
