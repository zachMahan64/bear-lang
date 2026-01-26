### Bear HIR Design Overview
```
---------------
|     key     |
=============== 
':' composed of
'|' or 
'->' mapped to
'[]' sliceof
'()' descript.
lowercase = 
    name
capitalized =
    HirType
---------------
```
```
RESOLUTION FLOW
---------------
- Declarations will define Hir nodes tied to scoped SymbolId tables and HirIdentifers will be the paths locating those definied nodes.
- Usages of any identifiers will explore those paths to ensure the existence of any mentioned identifiers by walking the scope tree and 
  then storing the HirId's at those the referenced Strings inside an HirIdentifier.
- Declarations must check their local scope to ensure no redefinitions occur.
   - hash look-up -> SymbolId representing the name (unscoped newly bound identifier) -> Map of the local HirScope pointing to the DefId
     of the newly declared node

CONTAINERS
=========================================================================
VECTOR TABLES (used like arenas, but memory is pointed to by ids/indices)
*persisent, must be serializable
-------------------------------------------------------------------------
note: the *IdIdx are necessary for contiguous slices of *Id's!
note: all Id Tables should reserve 0 to indicate an invalid Id!

HirSymbolIdIdx -> HirSymbolId
HirSymbolId -> HirSymbol

(*persisent, serializable)
HirScopeId->
    HirScope:
        parent: HirScopeId?
        ScopeTable:
        (these need to be uint32_t -> uint32_t hashmaps!)
            modules: SymbolId -> HirDefId     (namespace/module names/static struct members)
            values:  SymbolId -> HirDefId     (variables, functions)
            types:   SymbolId -> HirDefId     (structs, unions, deftypes, variants)

(temporary, for block scopes; not serialized)
HirScopeAnonId->
    HirScopeAnon:
        parent: HirScopeId
        anon_parent: HirScopeAnonId?
        table: ScopeTable
            
HirExecIdIdx-> HirExecId
HirExecId -> HirExec

HirDefIdIdx -> HirDefId
HirDefId -> HirDef

HirTypeIdIdx -> HirTypeId
HirTypeId -> HirType

HirGenericParamIdIdx -> HirGenericParamId 
HirGenericParamId -> HirGenericParam 

HirGenericArgIdIdx -> HirGenericArgId 
HirGenericArgId -> HirGenericArg

HASH TABLES, *temporary, not serialized
---------------------------------------
StringToSymbolTable: String → HirSymbolId

HIR NODE STRUCTURES
============================================================================

HirIdentifier (used in type, expr, and module references; eg: std..println):
    segments:
        start: HirSymbolIdIdx
        len: uint32_t
    span: Span
    resolved: DefId

HirExecId -> HirExec:
    span: Span
    | HirStatement:
        | HirBlockStmt:
            HirBody:
                locals: HirScopeAnonId
                execs: []HirExecId
        | HirExprStmt:
            HirExecId -> HirExpr
        | HirEmptyStmt:
            ()
        | HirBreakStmt:
            HirExecId? -> HirExpr
        | HirIfStmt:
            condition: HirExecId -> HirExpr
            body: HirExecId -> HirBlockStmt
        | HirElseStmt:
            body: HirExecId -> HirBlockStmt
        | HirLoopStmt:
            body: HirExecId -> HirBlockStmt
        | HirReturnStmt:
            HirExecId? -> HirExpr
        | HirYieldStmt:
            HirExecId? -> HirExpr
    | HirExpr:
        type: HirTypeId?   (filled after typecheck)
        kind: HirExprKind

HirExprKind:
    (atoms)
    | HirExprIdentifier:
        ident: HirIdentifier (resolved DefId inside)
    | HirExprLiteral:
        literal: LiteralValue
    (computed exprs)
    | HirExprListLiteral:
        elems: []HirExecId -> HirExpr
    | HirExprAssignMove:
        into: HirExecId 
        out_of: HirExecId
    | HirExprAssignEqual:
        to: HirExecId 
        from: HirExecId
    | HirExprIs 
        lhs: HirExecId 
        rhs: HirExecId -> HirExprVariantDecomp
    | HirExprBinary:
        op: BinaryOp
        left: HirExecId
        right: HirExecId 
    | HirExprCast
        expr: HirExecId
        target: HirTypeId
    | HirExprPreUnary:
        op: UnaryOp
        expr: HirExecId
    | HirExprPostUnary:
        op: PostUnaryOp
        expr: HirExecId
    | HirExprSubscript:
        base: HirExecId
        index: HirExecId
    | HirExprFnCall:
        callee: HirExecId
        args: []HirExecId
    | HirExprType:
        type: HirTypeId
    | HirExprBorrow:
        mutable: bool
        expr: HirExecId
    | HirExprStructInit:
        struct_def: HirDefId
        fields: []HirExecId -> HirExprStructMemberInit
    | HirExprStructMemberInit:
        field_def: HirDefId
        move_into: bool
        value: HirExecId
    | HirExprClosure:
        params: []HirParamId
        return_type: HirTypeId
        body: HirExecId -> HirBlockStmt
        captures: []HirDefId? (filled by capture analysis)
    | HirExprVariantDecomp:
        variant_def: HirDefId
        fields: []HirParamId?
    | HirExprBlock:
        block: HirExecId -> HirStmtBlock
    | HirExprMatch:
        scrutinee: HirExecId
        branches: []HirExecId -> HirMatchBranch
    HirMatchBranch:
        patterns: []HirExecId
        body: HirExecId
    | HirExprInvalid

(these are specifically type mentions, not definitions)
HirTypeId->
    HirType:
        span: Span
        | HirTypeBuiltin
        | HirTypeStructure:
            HirIdentifier
        | HirGenericStructure:
            HirTypeId
            []HirGenericArg
        | HirTypeArr:
            size: size_t? (non-optional after resolution)
            inner: HirTypeId
        | HirTypeRef:
            inner: HirTypeId
        | HirTypeFnPtr:
            []HirTypeId
            return_type: HirTypeId
        | HirTypeVariadic
            inner: HirTypeId

HirDefId ->
    span: Span
    name: SymbolId
    resolved: bool
    HirDef:
        | HirFunctionDef:
            []HirDefId -> HirParam
            return_type: HirTypeId
            HirExecId? -> HirBody
            orig: HirDefId? -> HirGenericFunctionDef (if was originally generic and then was generated)
        | HirParam:
            def: HirDefId? -> HirVarDef (non-optional after resolution)
        | HirGenericFunctionDef:
            []HirGenericParam 
            HirFunctionDef
        | HirDestructorDef:
            HirFunctionDef
        | HirExternCFunctionDef:
            HirFunctionDef
        | HirVarDef:
            type: HirTypeId
            name: SymbolId
        | MovedVarDef:
            orig: HirDefId
        | HirModDef:
            scope: HirScopeId
            name: SymbolId
        | HirStructDef
            scope: HirScopeId 
            name: SymbolId
            contracts: []HirDefId -> HirContractDef
            orig: HirDefId? -> HirGenericStructDef (if was originally generic and then was generated)
        | HirGenericStructDef
            HirStructDef
           []HirGenericParam 
        | HirVariantDef
            scope: HirScopeId 
            name: SymbolId
            orig: HirDefId? -> HirGenericVariantDef (if was originally generic and then was generated)
        | HirGenericVariantDef
            HirVariantDef
            []HirGenericParam
        | HirUnionDef
            scope: HirScopeId 
            name: SymbolId 
        | HirContractDef
            scope: HirScopeId 
            name: SymbolId 
        | HirDefTypeDef
            type: HirTypeId
            name: SymbolId 

HirGenericParam:
    | HirIdentifier (placeholder for types)
    | HirParam (placeholder for exprs)

HirGenericArg:
    | HirExecId -> HirExpr
    | HirTypeId

** note: generic defs will be stored as sugared and specifically instantiated as desugared defs that point to their orginal def

```

```
PASSES 
====== 

PHASE 1: PREDECLARE
--------------------
walk AST declarations only
allocate HirDefIds for:
- modules
- structs / unions / variants
- deftypes
- functions / externs functions
- global vars
- import: coalesce imports (build dependency tree?)

insert SymbolId -> HirDefId into current ScopeTable
create child HirScopes where needed
postpone building bodies, no identifiers resolved yet

result: all namespaces populated.

failure modes:
    REDEFINITION

PHASE 2: BUILD BODIES/RESOLVE
-----------------------------
walk AST again
build HIR for:
- function signatures/bodies
- struct / variant / union fields
- type syntax trees
- expressions / statements

create anonymous block scopes
insert local variables on sight

on identifier (type or variable reference):
- create HirIdentifier with resolved pointing a to a DefId by searching available scopes
- insert locals into local scope as defined and resolve naturally linearly

compile-time constructs:
    on generic(type structure or function):
        - attempt a concrete instantiation

    on compt value/array size/expression arg in generic:
        - ensure compile time computatbility (do simple checks for first resolver version)

failure modes: 
    | DNE 
    | REDEFINITION
    | CIRCULAR_DEPENDENCY
    -> emit errors

PHASE 3: TYPECHECK
----------------------------------
now walk HIR (since all DefIds fully resolved)
```
