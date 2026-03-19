bear-ls high-level plans/design
-------------------------------

##### go-to-def
-> binary search on linear node slices (statement slices) and recursively traverse outer spans containing the desired (line,col) until inside the innermost span, this is our symbol
-> track lexical scopes while traversing down for later reversed lookup
-> look up the given symbol inside the lexical scope (if that span did indeed correspond to a symbol) 
-> return the definition at that symbol
