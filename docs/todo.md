#### Short-term
- [x] cast parsing (decide between c-style vs as)
- [x] fn calls
- [x] expression-statement parsing
- [x] subscript expr parsing
- [x] proper type parsing (with dedicated ast_type_t nodes)
- [ ] parameter parsing when parsing functions
- [ ] generic type parsing
- [ ] other statement parsing

#### Medium-term
- [ ] Get a solid subset of the parser working on building an AST
- [ ] Write unit tests for the parser and work on a basic AST walker for debug logging (will also be helpful for semantic analysis and codegen)
- [ ] Revamp CLI to be much more robust using the strimap_t and vector_t containers
