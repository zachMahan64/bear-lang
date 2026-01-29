#### todos
- [x] add typed scope table accessor/mutator functions
- [x] add scope ctor & dtor
- [x] write the new file tracking system into tables.h 
- [ ] revise parser briefly to allow:
    - [ ] literal (or compt-expr?) initialized variant values
- [ ] finish hir structural impls
    - [ ] defs
    - [ ] types
    - [ ] exec
        - [ ] mapping from token operator token_type_e's -> hir-specific operator types 
    - [ ] generics args/params
- [ ] impl hir_tables_t for hir node allocation/access 
- [ ] hir phase 1: begin handling top level decl lowering
- [ ] hir phase 2: begin identifier resolution, typechecking, and constant folidng/compt analysis
#### Medium term
- [ ] HIR
- [ ] MIR? (compile-time ctrl flow and better optimizations) 
- [ ] LLVM IR
