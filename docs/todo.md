#### todos

main quest
----------

##### hir phase 2.a:
- all while in the process of *resolivng* top-level declarations:
- [x] fix `tests/hir/10.br` falsely reported circular
- [x] multi-line and more complicated diagnostics handling

- [x] `ast_expr_t*` lowering to `hir::Exec` (minimum constant folding/compt canonical value resolution)
    - [x] struct compt handling (propagate constants thru member inits)
    
    - [x] compt operators: string literal concat, basic operators for integral and floating values -> necessary for canonicalizing variable generic args w/ expressions
        - [x] implement kind of operator <--> type mapping system for at least builtins
        - [x] arithmetic
        - [x] string literal concat 
        - [x] bitwise
        - [x] boolean
        - [x] unary ops

    - [x] compt ternary if

    - [x] refactor compt expr solver for some readibility and factor out some common logic between components

    - [x] compt list literals
    
- [x] finish some more ContextDatabase functionality for better testing 

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
- [x] special expr case `@same_type(expr, expr)` (yields bool) where the types could be say `typeof(foo)` and `Foo<i32>`
- [x] static_assert (make it a builtin in hir)
    - perhaps `std..assert..static_assert(bool cond)` wrapping true builtin `@static_assert(bool cond)` 
- [x] `@type_to_str(<type>)` to get a reflected compt string that is just the type's string representation
    - since compt strs are just `hir::SymbolId`s just do `context.symbol_id(type_to_str(context, hir::TypeId))` to get them
- [x] add `fn foo() ~> i32` syntax for discardable return values

- [x] add `@defined()` expr

- [ ] some `compt` function, struct, and list improvements:
    - [x] add compt member accesses ex) `foo.bar`
    - [x] handle `pub` / `hid` statements properly when looking up member variables(/functions) 
    - [x] add `fn foo() [-> Type]? => [{Expr}] | Expr` syntax (easy-ish plugin for compt functions), this makes compt turing complete
        - ex) `compt fn foo() -> i32 => 42`
        - ex) `compt fn foo() => 42` (return type inferred here)
        - ex) `compt fn foo() => {42}` (return type inferred here, braces allowed if desired)
    - [x] add lowering of pure expr functions 
        - [x] ban `compt mt mut` funcs
        - [x] add compt member calls: `foo.bar()` 
    - [x] handle proper short-circuiting of `||` and `&&` at compt
    - [ ] generalize solving booleqs 
        - [ ] compt list equalities: `[1, 2] == [1, 2]` (elem type, then len, then elems)
        - [ ] struct equalities, at least ExecId-wise, for now 
    - [x] subscripts on compt lists and strings (bound check)
    - [ ] compt match: impl as chained comparisions ensuring each branch matches the type inside match(x)
        - [ ] handle the inline case syntax `cond | cond | cond`
        - [ ] make sure it's branches are exhaustive
            - [ ] for now, just make sure there's an `else` clause or that both `true`/`false` are covered
            - [ ] if feasible, add range checking

- [ ] implement generic args canonicalization to allow mapping of canonical lists of generic args to concrete instatiations for generic structs, variants, and functions
    - generic params become either deftypes to type args or simply compt variables for expression value args 

- [ ] **use canonical generic args canonicalization to memoize compt function args -> values**

- [ ] `ast_type_t*` lowering to for generic `hir::Type`s 
        - handle type deduction with `var` in decls: a `TypeInferer` allowing `var` to be decorated with `*`, `&`, etc, could be allowable with the `TypeTransformer` construct
    - [ ] A `TypeIsInferable` functor could be useful (this would allow decorated `var`s), just walk and match `TypeVar` with anything

- [ ] full `ast_stmt_t*` (top-level decls) lowering to `hir::Def` (requires both types and exprs)
    - [x] improve `use` statements to allow single-def usages in named scopes (not just modules in anon scopes)

- [ ] see and finish impl'ing the canonical generic args slice table outline, basically each canonical set of generi args for a given def needs to either:
    1. map to an already instatiated specialized, concrete instance of the def, or:
    2. instatiate a new defintion by lowering the `ast_stmt_t` after inserting the specific defintions for values into the scope 

- [ ] some basic [lsp-compt](/docs/lsp-compat.md), mostly thru building span -> scope search trees

- [ ] finish internal resolution logic on `hir::TopLevelVisitor` using all the lowering logic

##### hir phase 2.b:
- [ ] function body resolution 
- [x] handle `pub` / `hid` statements properly when looking up member variables(/functions) 
    - note: already working for arbitrarily scoped modules, types, and variables
- [ ] move checker

#### optimizations
- [ ] `hir::Context` ctor that takes a stale context and a list of update files, and then based on the stale context's files:
```
    for every file in stale context:
        if red: # stale and already marked as such 
            continue 
        if not red && stale: 
            color it and its dependents red
            continue 
        # since it's not stale:
        move file and it's data (buffer, token, ast) from stale context into new context

    delete the stale context and replace it with the new context 
    # note make sure dtor of files inside context properly handle being moved (no double frees, etc.)
```
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
- [ ] improve numerical literal handling 
    - [ ] probably just replace strtoll and friends with hand-rolled impls 
    - [ ] add binary integer literals `0b1010101` (keeping dec, hex, and float that we currently already have)
    - [ ] set a tkn to TOK_OVERSIZED_INT_ERR if there's no decimal and it's greater than u64 max or less than i64 min
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
- [x] make a scope iterator
- [ ] debug logger to display context and scope contents

##### lsp-friendly features
- [lsp compatibility plan here](docs/lsp-compat.md)
