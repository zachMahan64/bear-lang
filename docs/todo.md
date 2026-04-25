#### todos

main quest
----------

##### hir phase 2.a:
- all while in the process of *resolivng* top-level declarations:

- [ ] some more `compt` improvements:
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
    - [x] generalize solving booleqs 
        - [x] compt list equalities: `[1, 2] == [1, 2]` (elem type, then len, then elems)
        - [x] struct equalities
    - [x] subscripts on compt lists and strings (bound check)
    - [x] compt fn-pointers to allow compt fn composition 
    - [ ] compt match: impl as chained comparisions ensuring each branch matches the type inside match(x)
        - [ ] handle the inline case syntax `cond | cond | cond`
        - [ ] make sure it's branches are exhaustive
            - [ ] for now, just make sure there's an `else` clause or that both `true`/`false` are covered
            - [ ] if feasible, add range checking 
    - [ ] compt unions/variants
    - [ ] compt closures (pure-expr only) should naturally follow
        - [ ] allow capturing compt variables 


- [ ] implement generic args canonicalization to allow mapping of canonical lists of generic args to concrete instatiations for generic structs, variants, and functions
    - factor out `ComptExprSolver`'s equality logic (`ExecConst`, `ExecExprListInit`, and `ExecExprStructInit`) to use for comparing comparing compt execs inside the table
    - generic params become either deftypes to type args or simply compt variables for expression value args 
    - see and finish impl'ing the canonical generic args slice table outline, basically each canonical set of generi args for a given def needs to either:
        1. map to an already instatiated specialized, concrete instance of the def, or:
        2. instatiate a new defintion by lowering the `ast_stmt_t` after inserting the specific defintions for values into the scope 

- [ ] **use canonical generic args canonicalization to memoize compt function args -> values**

- [ ] `ast_type_t*` lowering to `hir::Type`s 
    - [x] non-generic types 
    - [ ] generic types (just find/instatiate mentioned generic def and then use that concrete def within the type)
    - [ ] handle type deduction with `var` in decls: a `TypeInferer` allowing `var` to be decorated with `*`, `&`, etc, could be allowable with the `TypeTransformer` construct
    - [ ] A `TypeIsInferable` functor could be useful (this would allow decorated `var`s), just walk and match `TypeVar` with anything 


- [ ] preliminary full `ast_stmt_t*` (top-level decls) lowering to `hir::Def` (requires both types and exprs)
    - [x] improve `use` statements to allow single-def usages in named scopes (not just modules in anon scopes)

- [ ] some basic [lsp-compat](/docs/lsp-compat.md), mostly thru building span -> scope search trees (only build these when a flag is enabled, tho; this will need to be added)

- [ ] finish internal resolution logic on `hir::TopLevelVisitor` using all the lowering logic for every possible `ast_stmt_t`
    - perhaps consider constexpr-based policies to exhaustively check all possibilities for more combinatorically complex stmts (fn_decls come to mind) 

##### hir phase 2.b:
- [ ] function body resolution 
- [x] handle `pub` / `hid` statements properly when looking up member variables(/functions) 
    - note: already working for arbitrarily scoped modules, types, and variables
- [ ] move checker
- [ ] a borrow checker should be trivial, however:
    - [ ] allow mutiple immutable and mutable borrows
    - [ ] no lifetimes
    - [ ] strictly ban returning a reference to a local variable
- [ ] remember: run-time values that are immutable references and have compile-time initializers can just reference static variables that store that compile-time value

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
- [ ] elipse out diagnostics after like 8 lines
- [ ] add trimming of cwd from file paths in diagnostics reporting?
- [ ] fully allow cyclical imports, add a flag to enable warnings instead of always warning for it
- [ ] using a scope iterator, use Levenshtein distance to make a `help: did you mean:` `...`

##### debugging 
- [x] make a scope iterator
- [ ] debug logger to display context and scope contents

##### lsp-friendly features
- [lsp compatibility plan here](docs/lsp-compat.md)
