#### todos
##### hir phase 1:
- [x] add typed scope table accessor/mutator functions
- [x] add scope ctor & dtor
- [x] write the new file tracking system into tables.h 
- [x] fix anonymous struct decls, make includes `__cplusplus` safe
- [x] C++ify HIR
- [x] Make Scope fully c++ified (finish translating Cisms)
- [x] Change scope to used std::vector since it's lazy
- [x] add an ArenaStringHashMap implementation for sybmbols (or consider an LLVM datastructure), see comment in hir/builder.cpp
- [x] flesh out query system a bit more, make a better ctor
- [x] improve CLI to accept import paths
- [x] finish hir structural impls
    - [x] defs
    - [x] types
    - [x] exec
        - [x] mapping from token operator token_type_e's -> hir-specific operator types 
    - [x] generics args/params
- [x] impl limited external query system for hir::Context
- [x] revamp to compiler_error_list_t to include diagnostics messages ("note")
- [x] finish impl top-level traversal for filling named scopes
    - [x] handle Foo..bar() function declaration (see TD comment in ast_visitor.cpp)
    - [x] make sure to set the parent field in defs (current the `OptId<DefId> parent` field is always none), this will later be important for canonicalizing definitons (especially types)
- [x] add better diagnostics (interned in HIR), which is file-wise, but track a `next_diag` chain and then store a `bool reported` to ensure no duplicate reporting
    - [x] add semantic diagnostics of various types, store metadata, and then write a stdout reporter (this will be seperate from tokenwise diagnostics). This will probably require dedicated DiagnosticId -> Diagnostic: Error | Warning | Noteo 
- [x] hir test cases
- [x] add a diagnostic type to handle circular imports (DiagnosticValue containing a slice of FileId should work)
- [x] properly set parent scopes during top-level traversal
- [x] handle proper ordering of struct fields in top level traversal (properly will need a dedicated function), achieve this by having `register_top_level_stmt`/`register_top_level_stmts` return `OptId<DefId>`/`IdSlice` where the OptId has a value only when the DefId is a var decl
- [ ] Type resolution requisites:
    - [x] add `TypeTransformer<T>` which is callable on 2 `TypeIds` (for recursive comparision) which is seeded with a functor `T` and a collected value `V` that takes in 2 `TypeId`'s, compares them, and returns some value based on that comparision (bool or some hash-component value will be helpful for typechecking and canonicalizing type hashing, respectively)
    - [x] CanonicalTypeHashTable, implement then add to context, (set canonical inside of Types and then also build the reverse map from CanonicalId -> first TypeId mention)
- [x] Fully plan out generic instatiation (current sketch in hir_design doc)
    - [x] the plan is to delay generic top level instatiation (2.a) until first mention (this is logical), and then resolve concrete-ified members in phase 2.b 

##### hir phase 2.a:
- all while in the process of *resolivng* top-level declarations:
- [x] fix `tests/hir/10.br` falsely reported circular
- [ ] multi-line and more complicated diagnostics handling
- [ ] `ast_expr_t*` lowering to `hir::Exec` (minimum constant folding/compt canonical value resolution)
    - [ ] implement basic ast-lowering for exprs constant folder/compt resolver -> string literal concat?, basic operators for integral and floating values -> necessary for canonicalizing variable generic args, not just types 
    - [ ] implement kind of operator <--> type mapping system (just worry about builtin types for now)
    - [ ] debug and increase safety of particularly (large) integral literal parsing
- [ ] `ast_type_t*` lowering to `hir::Type` (requires exprs for array subscripts and generic args)
- [ ] `ast_stmt_t*` (top-level decls) lowering to `hir::Def` (requires both types and exprs)
- [ ] allow for passing of `ast_generic_args_t*` for recursive generic instatiations in `hir::TopLevelVisitor`? or just insert into scope and pass thru the scope
    - some method of handling generic params -> concrete args forwarding inside the entire generic scope 
- [ ] finish internal resolution logic on `hir::TopLevelVisitor` using all the lowering logic
- [ ] debug logger to display context and scope contents
    - [ ] add a StringifyType seed for TypeTransformer
- [ ] hir phase 2: begin identifier resolution, typechecking, and constant folidng/compt analysis
#### medium term/non-main path tasks
- [x] `-o` / `--output` cli arg instead of implicit output
- [ ] std::string builder for hir::Type using `TypeTransformer`
#### long term
- [ ] MIR? (compile-time ctrl flow and better optimizations) 
- [ ] LLVM IR
