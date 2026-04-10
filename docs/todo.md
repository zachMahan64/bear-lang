#### todos

main quest
----------

##### hir phase 2.a:
- all while in the process of *resolivng* top-level declarations:
- [x] fix `tests/hir/10.br` falsely reported circular
- [x] multi-line and more complicated diagnostics handling

- [ ] `ast_expr_t*` lowering to `hir::Exec` (minimum constant folding/compt canonical value resolution)
    - [x] struct compt handling (propagate constants thru member inits)
    
    - [ ] compt operators: string literal concat, basic operators for integral and floating values -> necessary for canonicalizing variable generic args w/ expressions
        - [x] implement kind of operator <--> type mapping system for at least builtins
        - [x] arithmetic
        - [x] string literal concat 
        - [x] bitwise
        - [x] boolean
        - [x] unary ops

    - [ ] refactor compt expr solver to fully attempt an expr then type check instead of having type checking in both builtin and struct solving
    - [ ] compt list literals

    - [ ] compt ternary if
    
    - [ ] debug and increase safety of particularly (large) integral literal parsing

- [ ] finish some more ContextDatabase functionality for better testing 

- [x] if compt (in parser/syntax doc, hir impl will come later in body resolution)
- [x] ternary if: expr `if` `compt`? condition `else` expr (in parser/syntax doc)
- [x] top level use (in parser/syntax doc, hir logic)

- [x] **substantial refactor**: remove ScopeAnon, just make anonymous scopes have smaller default capacities (doing this since anon scopes should have their own namespace storage for used defs and i'm scrapping the old style anon uses)

- [x] implement Def::mention state in `hir::DefVisitor` thru:
    - `visit_as_independent(DefId)` -> `unmentioned` (add this method)
    - `visit_as_dependent(DefId)` & `visit_as_transparent(DefId)` -> `mentioned` (update these methods)
    - `visit_as_mutator(DefId)` -> `mutated` (add this method)

- [x] fix/impl deftypes by having a direct def -> type forward mechanism (will be needed for generic params too)

- some reflection/compt stuff
- [ ] `@type(...)` for getting type-exprs
- [ ] `same_type(expr, expr)` where the exprs could be say `typeof(foo)` and `@type(Foo<i32>)`
- [ ] static_assert (make it a builtin in hir)
    - perhaps `std..assert..static_assert(bool cond)` wrapping true builtin `__static_assert(bool cond)` 
- [ ] make `deftype = typeof(foo)` handling consistent

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
- [x] fix the `HashMap<i32, HashMap<i32, i32>>` parsing case by packing count tracking into `>>` and `>>>` tokens until they hit a value of 2/3
- [ ] binary integer literals `0b1010101`
- [ ] allow arbitrarily ordered struct members inits, will require mini symbol hashmaps
- [ ] add an Exec Stringifier (tedious)
- [ ] add a Def Stringifier (tedious)  
- [ ] arbitrary source code reconstruction from hir::Context

##### diagnostics
- [ ] elipse out diagnostics after like 16 lines
- [ ] add trimming of cwd from file paths in diagnostics reporting?
- [ ] fully allow cyclical imports? validate full compatibility, add a flag to enable warnings instead of always warning for it
- [ ] using a scope iterator, use Levenshtein distance to make a `help: did you mean:` `...`

##### debugging 
- [ ] make a scope iterator
- [ ] debug logger to display context and scope contents

##### lsp-friendly features
- [lsp compatibility plan here](docs/lsp-compat.md)
