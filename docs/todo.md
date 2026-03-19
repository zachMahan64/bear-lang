#### todos

#### chores 
- [x] handle alignas accordingly during lowering too

##### diagnostics
- [ ] add trimming of cwd from file paths in diagnostics reporting
- [ ] fully allow cyclical imports? validate full compatibility, add a flag to enable warnings for it
- [ ] using a scope iterator, use Levenshtein distance to make a did you mean... tip

##### debugging 
- [ ] make a scope iterator
- [ ] debug logger to display context and scope contents
- [ ] add a StringifyType seed for TypeTransformer

##### hir phase 2.a:
- all while in the process of *resolivng* top-level declarations:
- [x] fix `tests/hir/10.br` falsely reported circular
- [x] multi-line and more complicated diagnostics handling
- [ ] `ast_expr_t*` lowering to `hir::Exec` (minimum constant folding/compt canonical value resolution)
    - [ ] implement basic ast-lowering for exprs constant folder/compt resolver -> string literal concat?, basic operators for integral and floating values -> necessary for canonicalizing variable generic args, not just types
    - [ ] struct compt handling (propagate constants thru member inits)
    - [ ] implement kind of operator <--> type mapping system (just worry about builtin types for now)
    - [ ] debug and increase safety of particularly (large) integral literal parsing
- [ ] `ast_type_t*` lowering to `hir::Type` (requires exprs for array subscripts and generic args)
- [ ] `ast_stmt_t*` (top-level decls) lowering to `hir::Def` (requires both types and exprs)
- [ ] allow for passing of `ast_generic_args_t*` for recursive generic instatiations in `hir::TopLevelVisitor`? or just insert into scope and pass thru the scope
    - some method of handling generic params -> concrete args forwarding inside the entire generic scope 
- [ ] finish internal resolution logic on `hir::TopLevelVisitor` using all the lowering logic

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
    - At minimum, a regex-based syntax highlighter
    - basic cave/bearc integration (run button)

- [ ] Improve debugability 
