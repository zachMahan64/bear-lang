#### Short-term
- [x] cast parsing (decide between c-style vs as)
- [x] fn calls
- [x] expression-statement parsing
- [x] subscript expr parsing
- [x] proper type parsing (with dedicated ast_type_t nodes)
- [x] fix right assoc bug
- [x] add alignof
- [x] parameter parsing when parsing functions
- [x] array type parsing
- [x] generic type parsing
- [ ] pub/hid parsing for statements
- [ ] `...` and range/slice related syntax
- [ ] other statement parsing
- [ ] structs
- [ ] marks
- [ ] closures?
- [ ] decide between and impl switch and/or match 

#### Medium-term
- [ ] Get a solid subset of the parser working on building an AST
- [ ] Write unit tests for the parser and work on a basic AST walker for debug logging (will also be helpful for semantic analysis and codegen)
- [ ] Revamp CLI to be much more robust using the strimap_t and vector_t containers
