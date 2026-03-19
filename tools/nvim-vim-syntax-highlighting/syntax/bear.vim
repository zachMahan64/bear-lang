" syntax/bear.vim

if exists("b:current_syntax")
  finish
endif

syntax case match

" ------------------------
" identifiers
" ------------------------
syntax match bearIdentifier /\<[A-Za-z_][A-Za-z0-9_]*\>/
syntax match bearScopedIdentifier /\<[A-Za-z_][A-Za-z0-9_]*\(\.\.[A-Za-z_][A-Za-z0-9_]*\)\+\>/

" ------------------------
" keywords (from token map)
" ------------------------
syntax keyword bearKeyword
      \ import mod use fn mt dt
      \ mut contract requires
      \ var static extern compt hid pub has is
      \ if else while for return yield break continue match
      \ sizeof alignof typeof move as
      \ variant struct union deftype
      \ self Self
      \ in

" ------------------------
" types (primitive)
" ------------------------
syntax keyword bearType
      \ i8 u8 i16 u16 i32 u32 i64 u64 usize
      \ char f32 f64 str bool void

" ------------------------
" literals
" ------------------------

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
syntax match bearOperator /->\|=>\|<-\|<<-\|::\|\.\.\|<<\|>>\|>=\|<=\|==\|!=\|+=\|-=\|*=\|\/=\|%=\|&=\|\^=\|||\|&&/

" 1-char
syntax match bearOperator /\v[+\-*/%=&|!<>^~]/

" inc / dec / ellipsis
syntax match bearOperator /\v\+\+|--|\.\.\./

" ------------------------
" delims / punc
" ------------------------
syntax match bearDelimiter /[(){}\[\],;:#.]/

" ------------------------
" function / type names (heuristic)
" ------------------------

" fn name(
syntax match bearFunction /\<fn\s\+\zs[A-Za-z_][A-Za-z0-9_]*/

" dt name(
syntax match bearFunction /\<dt\s\+\zs[A-Za-z_][A-Za-z0-9_]*/

" mt [mut] name(
syntax match bearFunction /\<mt\s\+\(mut\s\+\)\?\zs[A-Za-z_][A-Za-z0-9_]*/

" struct / variant / union / contract names
syntax match bearTypeDef /\<\(struct\|variant\|union\|contract\)\s\+\zs[A-Za-z_][A-Za-z0-9_]*/

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

highlight default link bearKeyword Keyword
highlight default link bearType Type
highlight default link bearTypeDef Type
highlight default link bearBoolean Boolean

highlight default link bearIdentifier Identifier
highlight default link bearScopedIdentifier Identifier
highlight default link bearFunction Function

highlight default link bearNumber Number
highlight default link bearFloat Float
highlight default link bearString String
highlight default link bearChar Character

highlight default link bearOperator Operator
highlight default link bearDelimiter Delimiter

highlight default link bearTodo Todo
highlight default link bearComment Comment

let b:current_syntax = "bear"
