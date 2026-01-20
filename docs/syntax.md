### Bear syntax
- **Work In Progress:** This document will be the main syntax reference before a more formalized reference is created.

#### Grammar Diagrams
- Bear is comprised of a few distinct syntax constructs: Expressions, Statements, Clauses, and Types

#### Types
##### Basic built-in types:
- Signed integers: `i8, i16, i32, i64`
- Unsigned integers: `u8, u16, u32, u64, usize`
- Floating points: `f32, f64`
- Primitive strings: `str`
- Booleans: `bool`
- *TODO, finish*

#### Expressions
- Identifier: \[contains `A`-`Z`, `a`-`z`, `0`-`9`, and `_`; cannot begin with `0`-`9`] 
- ScopableIdentier: Identifier \[`..` Identifier...]?

- IntegerLiteral: \[follows C conventions for decimal and hexidecimal literals, minus suffixes]
- FloatingPointLiteral: \[follows C conventions for floating point literals, minus suffixes]
- StringLiteral: \[follows C conventions for string literals]

- ListLiteral: `[` Expression `,` Expression `,` ... `]`

- BinaryExpression: `Expression` BinaryOp `Expression`
    - BinaryOp: *TODO, write out precedence table* 
- Preunary: `--` | `++` Expression
- SizeOf: `sizeof` `(` Type `)`
- AlignOf: `alignof` `(` Type `)`
- TypeOf: `typeof` Expression
- Postunary: *TODO, write this out*
- Subscript: Expression `[` Expression `]`
- Grouping: `(` Expression `)`
- FunctionCall: Expression `(` Argument(s) `)`
    - Arguments: Expression `,` Expression `,` ...
- Borrow: `&` `mut`? Expression
- Dereference: `*` Expression
- StructInitialization: Identifier `{` StructFieldInitialization(s) `}`
    - StructFieldInitialization: `.` Identifier `=` | `<-` Expression 
- Closure: `move`? `|` Parameter(s) `|` `{` BodyStatement(s) `}`
- VariantDecomposition: Identifier `(` Parameter(s) `)`
- *TODO, finish*

#### Statements
- *File*: TopLevelStatement(s)

- *TopLevelStatement*: ModuleDeclaration | ImportStatement | FunctionDeclaration | StructDeclaration | VariantDeclaration | ContractDeclaration | UnionDeclaration | ExternBlock | DefTypeDeclaration | VariableDeclaration | UseStatement

- *ModuleDeclaration*: `mod` Identifier `;` | `{` TopLevelStatement(s) `}` 
- ImportStatement: `import` ExternalLanguage? -> PathLiteral -> \[`->` -> ModuleIdentifier]? -> `;` 
- *FunctionDeclaration*: `fn` Identifier -> GenericParams? -> `(` Parameters `)` `;` | `{` BodyStatement(s) `}`
- GenericParams: `<` Identifier -> HasClause? `,` ... `>` 
- *StructDeclaration*: `struct` Identifier -> GenericParams? -> HasClause? -> `{` TopLevelStatement(s) | MethodDeclaration(s) | DestructorDeclaration(s) `}` 
    - *MethodDeclaration*: `mt` `mut`? Identifier -> GenericParams? -> `(` -> Parameter(s) -> `)` -> `;` | `{` BodyStatement(s) `}`
    - *DestructorDeclaration*: `dt` `self` GenericParams? -> `(` -> Parameters -> `)` -> `;` | `{` BodyStatement(s) `}`
- *HasClause*: `has` `(` Contract(s) `)`
- *VariantDeclaration*: `variant` Identifier -> GenericParams? `{` VariantFieldDeclaration(s) `}`
- *VariantFieldDeclaration*: Identifier `(` Parameter(s) `)`
- *ContractDeclaration*: `contract` Identifier `{` MethodPrototypes `}`
- *UnionDeclaration*: `union` Identifier `{` MemberVariableDeclaration(s) `}`
- *ExternBlock*: `extern` ExternalLanguage `{` -> FunctionDeclaration(s) `}`
- *DefTypeDeclaration*: `deftype` Identifier `=` Type `;`
- *VariableDeclaration*: Type | `var` Identifier `;` | \[`=` | `<-`] Expression `;`
- *UseStatement*: `use` Identifier `;`

- *BodyStatement*: FunctionDeclaration | BlockStatement | IfStatement | WhileStatement | ForLoopStatement | ForInLoopStatement | VariableDeclaration | ReturnStatement | EmptyStatement | UseStatement | ExpressionStatement
- *LoopBodyStatement*: BodyStatement | BreakStatement

- *BlockStatement*: `{` BodyStatement(s) `}`
- *IfStatement*: `if` Expression `{` BodyStatement(s) `}` \[`else` `{` BodyStatement(s) `}`]?
- WhileStatement: `while` Expression `{` LoopBodyStatement(s) `}`
- ForLoopStatement: `for` `(` BodyStatement `;` Expression `;` Expression `)` `{` LoopBodyStatement(s) `}`
- ForInLoopStatement: `for` `var` Identifier `in` Expression `{` LoopBodyStatement(s) `}`
- ReturnStatement: `return` Expression? `;`
- EmptyStatement: `;`
- ExpressionStatement: Expression `;`
- BreakStatement: `break` `;`

