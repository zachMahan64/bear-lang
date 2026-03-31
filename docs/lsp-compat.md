lsp compatibility plan 
---------------------- 
- example request:
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "textDocument/definition",
  "params": {
    "textDocument": {
      "uri": "file:///home/user/project/src/main.br"
    },
    "position": {
      "line": 14,
      "character": 8
    }
  }
}
```
- note: line and character are 0-indexed

inside `hir::Context`
--------------------

- URI for file paths is completely handled by FileIds (file and ast interning system)

incoming `{line, character}` request handling
---------------------------------------------
- [ ] add tracking of `hir::FileId` -> `BinaryTree<{Span, ScopeId}, Comparator>` **arena-backed** where each source file tracks an ordered mapping of spans to scopes 
    - just do this tracking every time a scope (named or unnamed) is made, DNE assumes top level
    - this tracking should probably be toggled off during normal compilation 

- [ ] handle turning request into a parsable symbol
    - given a line/character re-tokenization will be necssary (walk forward/backward to try to find a valid symbol)
    - needs to consider encoding (utf-8/utf-16), this will require rewalking the entire line to map the encoded character idx into compiler ASCII character idx
    - the current lexer can probably be entirely reused for the actual tokenization if refactored, but this may not be necssary if we're just looking for symbols (regex or manual regex equiv is fine for scoped symbols)
    - arbitrary re-lexing of a given line would allow arbitrary reparsing for small snippets, thus allowing massively faster Context queries.

outgoing `{line, character}` diagnostics handling
------------------------------------------------- 
- already cleanly handled with diagnostics that track source Spans that contain `{FileId, start, len line, col}` which can easily be converted to `{uri, line, col, len}`
    - any necssary info can be attached to diagnostics at those locations 
