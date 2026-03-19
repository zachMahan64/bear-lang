" see README.md for set-up

if exists("b:current_syntax")
  finish
endif

syntax case match

" ------------------------
" identifiers
" ------------------------
"
" regular, only matches if it starts with lowercase or underscore
syntax match bearIdentifier /\<[a-z_][A-Za-z0-9_]*\>/

" scoped
syntax match bearScopedIdentifier /\<[A-Za-z_][A-Za-z0-9_]*\(\.\.[A-Za-z_][A-Za-z0-9_]*\)\+\>/

" ------------------------
" keywords (from token map)
" ------------------------
syntax keyword bearInclude
      \ import

syntax keyword bearStorageClass
      \ static extern compt hid pub mut

syntax keyword bearStructure
      \ variant struct union deftype

syntax keyword bearStatement
      \ return yield break continue

syntax keyword bearConditional
      \ if else match

syntax keyword bearRepeat
      \ while for in

syntax keyword bearMacro
      \ sizeof alignof typeof alignas

syntax keyword bearKeyword
      \ mod use fn mt dt
      \ contract requires
      \ has is move as
      \ self Self

syntax keyword bearSpecial var 

" ------------------------
" types (primitive)
" ------------------------
syntax keyword bearType
      \ i8 u8 i16 u16 i32 u32 i64 u64 usize
      \ char f32 f64 str bool void

" ------------------------
" literals
" ------------------------
"
" boolean + null
syntax keyword bearBoolean true false null

" integer (dec + hex)
syntax match bearNumber /\<0x[0-9A-Fa-f]\+\>/
syntax match bearNumber /\<[0-9]\+\>/

" float
syntax match bearFloat /\<[0-9]\+\.[0-9]\+\([eE][+-]\?[0-9]\+\)\?\>/

" char literal (handles escapes roughly)
syntax match bearChar /'\([^'\\]\|\\.\)'/

" string literal
syntax region bearString start=/"/ skip=/\\"/ end=/"/

" ------------------------
" operators
" ------------------------
" 4-char
syntax match bearOperator /\v>>>=/

" 3-char
syntax match bearOperator /\v<<=|>>=|>>>|\.\.\.=/

" 2-char
syntax match bearOperator /->\|<<-\|::\|\.\.\|<<\|>>\|>=\|<=\|==\|!=\|+=\|-=\|*=\|\/=\|%=\|&=\|\^=\|||\|&&/

" 1-char
syntax match bearOperator /\v[+\-*/%=&|!<>^~]/

" inc / dec / ellipsis
syntax match bearOperator /\v\+\+|--|\.\.\./

" highlight <- and => operators since they're more syntactically significant
syntax match bearSpecial /=>\|<-/

" ------------------------
" delims / punc
" ------------------------
syntax match bearDelimiter /[(){}\[\],;:#.]/

" ------------------------
" function / type names
" ------------------------

" foo(...
syntax match bearFunction /\<[a-z_][A-Za-z0-9_]*\ze\s*(/

" ------------------------
" special id styles
" ------------------------

" PascalCase: starts with A-Z, followed by any alphanumeric/underscore
syntax match bearPascalType /\<[A-Z][A-Za-z0-9_]*\>/

" CONSTANT_CASE: matches if the whole word is Uppercase/Numbers/Underscore
syntax match bearConstant /\<[A-Z][A-Z0-9_]*\>/


" ------------------------
" todo & fixme 
" ------------------------
syntax keyword bearTodo contained TODO FIXME XXX NOTE

" ------------------------
" comments (ONLY //)
" ------------------------
syntax match bearComment /\/\/.*/ contains=bearTodo,@Spell

" ------------------------
" highlight links
" ------------------------
highlight default link bearInclude     Include
highlight default link bearStorageClass StorageClass
highlight default link bearStructure   Structure
highlight default link bearStatement   Statement
highlight default link bearConditional Conditional
highlight default link bearRepeat      Repeat
highlight default link bearMacro       Macro
highlight default link bearSpecial     Special
highlight default link bearKeyword     Keyword
highlight default link bearType        Type
highlight default link bearTypeDef     Type
highlight default link bearBoolean     Boolean
highlight default link bearConstant    Constant
highlight default link bearPascalType  Type
highlight default link bearIdentifier  Identifier
highlight default link bearScopedIdentifier Identifier
highlight default link bearFunction    Function
highlight default link bearOperator    Operator
highlight default link bearDelimiter   Delimiter
highlight default link bearNumber      Number
highlight default link bearFloat       Float
highlight default link bearString      String
highlight default link bearChar        Character
highlight default link bearTodo        Todo
highlight default link bearComment     Comment
let b:current_syntax = "bear"

