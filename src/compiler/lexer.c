/*
lexer pseudo code:
==================
if (text.legth == 1):
    push back into token vector from a sigle char to token array look up table (no hashmap needed)
elif (hashmap contains sym's text):
    push back token from hashmap
elif (curr scopes' symbol_table contains symbol):
    push back token from symbol table
    else: error, unknown symbol
*/
