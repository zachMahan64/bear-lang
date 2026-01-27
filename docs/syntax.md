### Bear syntax
- **Work In Progress:** This document is the primary grammar reference for Bear.

#### Grammar Diagrams
- Bear is comprised of a few distinct syntax constructs: Expressions, Statements, Types, (and Clauses, which are subcomponents of certain statements)

#### Types
- Type: 
    - MutType: `mut`? Type `mut`? *** `mut`s bind leftward, unless nothing is to the left, where they will then bind rightward
    - | FnPtr: `*` `fn` `mut`? `(` Parameter(s) `)` `->` ReturnType(s)
    - | BaseType:
        - Signed integers: `i8, i16, i32, i64`
        - | Unsigned integers: `u8, u16, u32, u64, usize`
        - | Floating points: `f32, f64`
        - | Primitive strings: `str`
        - | Boolean: `bool`
        - | Variant: (implicit: `variant`) Identifier
        - | Struct: (implicit: `struct`) Identifier
        - | Union: (implicit: `union`) Identifier
    - | Generic: \[BaseType `::`? `<` GenericParam(s) `>`] | BaseType `::` GenericParam
    - | Array: `[` CompileTimeExpression `]` Type 
    - | Slice: `[` `&` `mut`? `]` Type
    - | Reference: Type `&`
    - | Pointer: Type `*`
    - | Variadic: Type `...`

#### Expressions
- Expression:
    - Identifier: \[contains `A`-`Z`, `a`-`z`, `0`-`9`, and `_`; cannot begin with `0`-`9`] 
    - | ScopableIdentier: Identifier \[`..` Identifier...]?
    - | IntegerLiteral: \[follows C conventions for decimal and hexidecimal literals, minus suffixes]
    - | FloatingPointLiteral: \[follows C conventions for floating point literals, minus suffixes]
    - | StringLiteral: \[follows C conventions for string literals]
    - | ListLiteral: `[` Expression `,` Expression `,` ... `]`
    - | BinaryExpression: `Expression` BinaryOp `Expression`
        - BinaryOp: a full precedence chart is in the works, for now refer to the maps in the [parser rules file](bearc/src/compiler/parser/rules.c) 
    - | Preunary: `--` | `++` | `typeof` Expression
    - | SizeOf: `sizeof` `(` Type `)`
    - | AlignOf: `alignof` `(` Type `)`
    - | Postunary: Expression `++` | `--`
    - | Subscript: Expression `[` Expression `]`
    - | Grouping: `(` Expression `)`
    - | FunctionCall: Expression `(` Argument(s) `)`
        - Arguments: Expression `,` Expression `,` ...
    - | Borrow: `&` `mut`? Expression
    - | Dereference: `*` Expression
    - | StructInitialization: Identifier `{` StructFieldInitialization(s) `}`
       - StructFieldInitialization: `.` Identifier `=` | `<-` Expression 
    - | Closure: `move`? `|` Parameter(s) `|` `{` BodyStatement(s) `}`
    - | VariantDecomposition: Identifier `(` Parameter(s) `)`
    - | Match: `match` `(` Expression `)` `{` MatchBranch(s) `}`
    - | MatchBranch: VariantDecomposition | Expression `=>` Expression | BlockExpression
       - BlockExpression: `{` Statement(s) | YieldStatement(s) `}` 
            - YieldStatement: `yield` Expression

#### Statements
- *File*: TopLevelStatement(s)
- Visibility: \[`pub` | `hid`]?
- *TopLevelStatement*: 
    - | *ModuleDeclaration*: Visibility `mod` Identifier `;` | `{` TopLevelStatement(s) `}` 
    - | ImportStatement: `import` ExternalLanguage? -> PathLiteral -> \[`->` -> ModuleIdentifier]? -> `;` 
    - | *FunctionDeclaration*: Visibility `fn` Identifier -> GenericParams? -> `(` Parameters `)` `;` | `{` BodyStatement(s) `}`
    - | GenericParams: `<` Identifier -> HasClause? `,` ... `>` 
        - *HasClause*: `has` `(` Contract(s) `)`
    - | *StructDeclaration*: Visibility `struct` Identifier -> GenericParams? -> HasClause? -> `{` TopLevelStatement(s) | MethodDeclaration(s) | DestructorDeclaration(s) `}` 
    - | *MethodDeclaration*: Visibility `mt` `mut`? Identifier -> GenericParams? -> `(` -> Parameter(s) -> `)` -> `;` | `{` BodyStatement(s) `}`
    - | *DestructorDeclaration*: `dt` `self` GenericParams? -> `(` -> Parameters -> `)` -> `;` | `{` BodyStatement(s) `}`
    - | *VariantDeclaration*: Visibility `variant` Identifier -> GenericParams? `{` VariantFieldDeclaration(s) `}`
    - | *VariantFieldDeclaration*: Identifier `(` Parameter(s) `)`
    - | *ContractDeclaration*: Visibility `contract` Identifier `{` MethodPrototypes `}`
    - | *UnionDeclaration*: Visibility `union` Identifier `{` MemberVariableDeclaration(s) `}`
    - | *ExternBlock*: `extern` ExternalLanguage `{` -> FunctionDeclaration(s) `}`
    - | *DefTypeDeclaration*: Visibility `deftype` Identifier `=` Type `;`
    - | *TopLevelVariableDeclaration*: Visibility \[Type | `var`] Identifier `;` | \[`=` | `<-`] Expression `;`

- *BodyStatement*: 
    - | VariableDeclaration: Type | `var` Identifier `;` | \[`=` | `<-`] Expression `;`
    - | *UseStatement*: `use` Identifier `;`
    - | BlockStatement: `{` BodyStatement(s) `}`
    - | IfStatement: `if` Expression `{` BodyStatement(s) `}` \[`else` `{` BodyStatement(s) `}`]?
    - | WhileStatement: `while` Expression `{` LoopBodyStatement(s) `}`
    - | ForLoopStatement: `for` `(` BodyStatement `;` Expression `;` Expression `)` `{` LoopBodyStatement(s) `}`
    - | ForInLoopStatement: `for` `var` Identifier `in` Expression `{` LoopBodyStatement(s) `}`
    - | ReturnStatement: `return` Expression? `;`
    - | EmptyStatement: `;`
    - | ExpressionStatement: Expression `;`

- *LoopBodyStatement*: 
    - BodyStatement
    - | BreakStatement: `break` `;`


