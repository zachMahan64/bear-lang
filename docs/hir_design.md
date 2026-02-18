### Bear HIR Design Overview
```
---------------
|     key     |
=============== 
':' composed of
'|' or 
'->' mapped to
'#>' hashmapped
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
   - hash look-up -> SymbolId representing the name (unscoped newly bound identifier) -> Map of the local HirScope pointing to the DefId of the newly declared node

CONTAINERS
=========================================================================
VECTOR TABLES (used like arenas, but memory is pointed to by ids/indices)
*persisent, must be serializable
-------------------------------------------------------------------------
note: the *IdIdx are necessary for contiguous slices of *Id's!
note: all Id Tables should reserve 0 to indicate an invalid Id!

HirSymbolIdIdx -> HirSymbolId
HirSymbolId ->
    HirSymbol:
        strv: string_view_t (into arena-backed storage)

HirFileIdIdx -> HirFileId
HirFileId -> 
    HirFile:
        path: HirSymbolId
        ast: HirAstId
(for tracking dependencies during phases 2, so necessary files can be re-entered)
dependencies_forward: []HirFileId (importer -> importees)
(for tracking dependencies for recompilation)
dependencies_reverse: []HirFileId (imported -> importers)

AstId -> Ast

(*persisent, serializable)
HirScopeId->
    HirScope:
        parent: HirScopeId?
        ScopeTable:
        (these need to be uint32_t -> uint32_t hashmaps!)
            modules: SymbolId #> HirDefId     (namespace/module names/static struct members)
            values:  SymbolId #> HirDefId     (variables)
            funcs:  SymbolId #> HirDefId      (functions)
            types:   SymbolId #> HirDefId     (structs, unions, deftypes, variants)

(unless top-level: temporary, for block scopes, not serialized)
HirScopeAnonId->
    HirScopeAnon:
        named_parent: HirScopeId?
        anon_parent: HirScopeAnonId?
        table: ScopeTable
        used_defs: []HirDefId
        has_used_defs: bool
        is_top_level: bool
            
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
StringToSymbolTable: String #> HirSymbolId
FilePathTable: HirSymbolId #> HirFileId

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
    compt: bool (literals implicity compt) 
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
            ()
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
        type: HirTypeId
        kind: HirExprKind

HirExprKind:
    (atoms)
    | HirExprIdentifier:
        ident: HirIdentifier (resolved DefId inside)
    | HirExprLiteral:
        LiteralValue:
            | HirIntegralLit
            | HirFloatingLit
            | HirStringLit
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
    | HirExprBorrow:
        mutable: bool
        expr: HirExecId
    | HirExprDeref:
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
        move: bool
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
        | HirTypeSlice:
            inner: HirTypeId 
        | HirTypeRef:
            inner: HirTypeId
        | HirTypePtr:
            inner: HirTypeId
        | HirTypeFnPtr:
            []HirTypeId
            return_type: HirTypeId
        | HirTypeVariadic
            inner: HirTypeId

HirDefId ->
    HirDef:
        span: Span
        name: SymbolId
        parent: HirDefId?
        resolved: bool
        top_leveL_visited: bool // prevent and detect circular dependencies
        pub: bool
        | HirFunctionDef:
            []HirDefId -> HirParam
            return_type: HirTypeId
            compt: bool
            HirExecId? -> HirBody
            orig: HirDefId? -> HirGenericFunctionDef (if was originally generic and then was generated)
        | HirGenericFunctionDef:
            []HirGenericParam 
            HirFunctionDef
        | HirDefFunctionPrototype:
            params: DefId?
            return_type: TypeId?
            language: hir::extern_lang
        | HirVarDef:
            type: HirTypeId
            name: SymbolId
            is_static: bool (implies pinned too)
            moved: bool
            {
            compt: bool
            constant: HirExecId? -> HirExprLiteral
            }
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
        | HirVariantDefField
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
- import: coalesce imports, build forward & reverse file dependency tree

insert SymbolId -> HirDefId into current ScopeTable
create child HirScopes where needed
postpone building bodies, no identifiers resolved yet

result: all namespaces populated (with corresponding HirScopes).

failure modes:
    REDEFINITION

PHASE 2: RESOLVE SIGNATURES -> BODIES
-------------------------------------
a)
walk AST again
build HIR with DFS (with circularity checks) for:
- function signatures
- struct / variant / union fields
- type syntax trees inside fields/params/return types
- place bodies to be populated into a vector of the form {HirDefId, AstNode} to be 
later iteratively resolved without fully rewalking the AST

b)
- bodies { types / expressions (typecheck) / statements (typecheck) }

create anonymous block scopes
insert local variables on sight

on identifier (type or variable reference):
- create HirIdentifier with resolved pointing a to a DefId by searching available scopes
- insert locals into local scope as defined and resolve naturally linearly

- types must be hashed and interned for canonical identification (especially helpful for generics)
    - recursive hashing thru SymbolId's for basetypes and then specific value for ref, ptr, arr, etc, should be sufficient
    - make sure to consider namespaces when canonicalizing 

compile-time constructs:
    on generic(type structure or function):
        - attempt a concrete instantiation

    on compt value/array size/expression arg in generic:
        - ensure compile time computatbility (do simple visiting checks for first resolver version)
        - recursive visiting nodes and checking compt all the way down will be necessary  

failure modes: 
    | DNE 
    | REDEFINITION
    | CIRCULAR_DEPENDENCY/SELF_REFERENTIAL_STRUCT
    -> emit errors

```
