#### todos

main quest
----------

##### hir phase 2.a:
- all while in the process of *resolivng* top-level declarations:
- [x] fix `tests/hir/10.br` falsely reported circular
- [x] multi-line and more complicated diagnostics handling

- [ ] `ast_expr_t*` lowering to `hir::Exec` (minimum constant folding/compt canonical value resolution)
    - [x] struct compt handling (propagate constants thru member inits)
    
    - [ ] compt operators: string literal concat?, basic operators for integral and floating values -> necessary for canonicalizing variable generic args w/ expressions
        - [ ] implement kind of operator <--> type mapping system for at least builtins

    - [ ] compt list literals

    - [ ] debug and increase safety of particularly (large) integral literal parsing

- [ ] implement generic args canonicalization to allow mapping of canonical lists of generic args to concrete instatiations for generic structs, variants, and functions 

- [ ] `ast_type_t*` lowering to `hir::Type` (requires exprs for array subscripts and generic args)
        - handle type deduction with `var` in decls: a `TypeInferer` allowing `var` to be decorated with `*`, `&`, etc, could be allowable with the `TypeTransformer` construct
    - [ ] A `TypeIsInferable` functor could be useful (this would allow decorated `var`s)

- [ ] `ast_stmt_t*` (top-level decls) lowering to `hir::Def` (requires both types and exprs)
    - [ ] improve `use` statements to allow single-def usages in named scopes (not just modules in anon scopes)

- [ ] allow for passing insertion of `ast_generic_args_t*` -> into scope (variables and types)

- [ ] properly handle default field values in structs   

- [ ] see and finish impl'ing the canonical generic args slice table outline, basically each canonical set of generi args for a given def needs to either:
    1. map to an already instatiated specialized, concrete instance of the def, or:
    2. instatiate a new defintion by lowering the `ast_stmt_t` after inserting the specific defintions for values into the scope 

- [ ] [lsp-compt](/docs/lsp-compat.md), mostly thru building span -> scope search trees

- [ ] finish internal resolution logic on `hir::TopLevelVisitor` using all the lowering logic

##### hir phase 2.b:
- [ ] function body resolution 
- [ ] handle `pub` / `hid` statements properly when looking up member variables(/functions) 
    - note: already working for arbitrarily scoped modules, types, and variables
- [ ] move checker

#### long term (compiler)
- [ ] MIR? (compile-time ctrl flow and better optimizations)
- [ ] LLVM IR (most likely lower directly from HIR)

#### tools 
- [ ] cave package-manager
    - run, init, build, check, and other nice-to-haves

- [ ] language server 
    - using the LSP (for VSCode/IDE/text-editor portability)
    - implemented in C++ (or Rust) using libbearc

- [ ] bear-tree-sitter
    - parser implemented with tree-sitter for complex syntax highlighing 

- [ ] VSCode
    - [x] At minimum, a regex-based syntax highlighter
    - [ ] basic cave/bearc integration (run button)

- [ ] Improve debugger compatibility 

side quests
-----------

##### chores 
- [x] handle alignas accordingly during ast lowering
- [ ] fix the `HashMap<i32, HashMap<i32, i32>>` parsing case by packing count tracking into `>>` and `>>>` tokens until they hit a value of 0. Will be a bit a tricky but it's doable. Needed in just generic args parsing
- [ ] allow arbitrarily ordered struct members inits, will require mini symbol hashmaps
- [ ] add an Exec Stringifier (tedious)
- [ ] add a Def Stringifier (tedious)  
- [ ] arbitrary source code reconstruction from hir::Context

##### diagnostics
- [ ] elipse out diagnostics after like 16 lines
- [ ] add trimming of cwd from file paths in diagnostics reporting?
- [ ] fully allow cyclical imports? validate full compatibility, add a flag to enable warnings for it
- [ ] using a scope iterator, use Levenshtein distance to make a `help: did you mean:` `...`

##### debugging 
- [ ] make a scope iterator
- [ ] debug logger to display context and scope contents

##### lsp-friendly features
- [lsp compatibility plan here](docs/lsp-compat.md)
