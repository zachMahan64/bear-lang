// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "compiler/lexer.h"
#include "compiler/errors/error_list.h"
#include "compiler/errors/error_messages.h"
#include "containers/vector.h"
#include "log.h"
#include "token.h"
#include <stddef.h>

/**
 * create a vector storing token_t from a specified src_buffer_t
 *
 */
vector_t lexer_tokenize_src_buffer(const src_buffer_t* buf) {
    vector_t tkn_vec =
        vector_create_and_reserve(sizeof(token_t), buf->size / LEXER_ESTIMATED_CHARS_PER_TOKEN);

    // tkn string view params
    char* start = buf->data;               // start of tkn's view into buf
    size_t len = 0;                        // len of tkn's view into buf
    src_loc_t loc = {.line = 0, .col = 0}; // location in src file
    size_t col = 0;
    token_t tkn; // scratch token

    char* pos = buf->data;
    const char* end_of_buf = buf->data + buf->size;
    char c;  // cached curr char, pos[0]
    char n1; // cached next char for lookaheads, pos[1]

    // maps
    const char* always_one_char_map = get_always_one_char_to_token_map();
    const char* first_char_in_multichar_operator_token_map =
        get_first_char_in_multichar_operator_token_map();

// pushes any previous token and delimits by pushing new token of a known length N in chars
#define LEX_KNOWN_LEN_PUSH(N)                                                                      \
    do {                                                                                           \
        if (len != 0) {                                                                            \
            tkn = token_build(start, len, &loc);                                                   \
            vector_push_back(&tkn_vec, &tkn);                                                      \
            len = 0;                                                                               \
            loc.col = col;                                                                         \
            start = pos;                                                                           \
        }                                                                                          \
        tkn = token_build(start, N, &loc);                                                         \
        vector_push_back(&tkn_vec, &tkn);                                                          \
        pos += (N);                                                                                \
        col += (N);                                                                                \
        loc.col = col;                                                                             \
        start = pos;                                                                               \
        len = 0;                                                                                   \
        goto lex_start;                                                                            \
    } while (0)

#define LEX_IN_LITERAL(D)                                                                          \
    ++pos;                                                                                         \
    ++len;                                                                                         \
    ++col;                                                                                         \
    while (true) {                                                                                 \
        c = *(++pos);                                                                              \
        ++len;                                                                                     \
        ++col;                                                                                     \
        if (c == '\n') {                                                                           \
            tkn = token_build(start, len, &loc);                                                   \
            vector_push_back(&tkn_vec, &tkn);                                                      \
            len = 0;                                                                               \
            loc.col = 0;                                                                           \
            ++loc.line;                                                                            \
            start = pos;                                                                           \
            break;                                                                                 \
        }                                                                                          \
        if (pos - 1 >= buf->data && c == (D) && *(pos - 1) != '\\') {                              \
            ++len;                                                                                 \
            tkn = token_build(start, len, &loc);                                                   \
            vector_push_back(&tkn_vec, &tkn);                                                      \
            len = 0;                                                                               \
            loc.col = 0;                                                                           \
            ++loc.line;                                                                            \
            ++pos;                                                                                 \
            start = pos;                                                                           \
            break;                                                                                 \
        }                                                                                          \
    }                                                                                              \
    goto lex_start;

lex_start:
    c = *pos;
    if (always_one_char_map[(unsigned char)c]) {
        LEX_KNOWN_LEN_PUSH(1);
    }
    if (c == '/' && (pos + 1 < end_of_buf) && pos[1] == '/') {
        goto lex_inline_comment;
    }
    if (first_char_in_multichar_operator_token_map[c]) {
        goto lex_multichar_operator;
    };
    if (c == ' ' || c == '\t') {
        goto lex_whitespace;
    }
    if (c == '\n') {
        goto lex_newline;
    }
    if (c == '\'') {
        LEX_IN_LITERAL('\'');
    }
    if (c == '\"') {
        LEX_IN_LITERAL('\"');
    }

    if (c == '\0') {
        goto lex_end;
    }
    ++pos;
    ++len;
    ++col;
    goto lex_start;

lex_multichar_operator:
    if (pos + 1 < end_of_buf) {
        n1 = pos[1]; // cache
    } else {
        n1 = '\0'; // makes future n1 bounds check unnecessary
    }
    switch (c) {
    case ('.'): {
        if (end_of_buf && n1 == '.') {
            LEX_KNOWN_LEN_PUSH(2);
        }
        if ((n1) >= '0' && n1 <= '9') {
            // just proceed, this is a float lit
            ++pos;
            ++len;
            ++col;
            goto lex_start;
        }
        LEX_KNOWN_LEN_PUSH(1);
    }
    // assignment
    case ('='): {
        if (n1 == '=') {
            LEX_KNOWN_LEN_PUSH(2);
        }
        LEX_KNOWN_LEN_PUSH(1);
    }

    // arithmetic
    case ('+'): {
        if (n1 == '=' || n1 == '+') {
            // ++ or +=
            LEX_KNOWN_LEN_PUSH(2);
        }
        LEX_KNOWN_LEN_PUSH(1);
    }
    case ('-'): {
        if (n1 == '=' || n1 == '-' || n1 == '>') {
            // --, ->, or -=
            LEX_KNOWN_LEN_PUSH(2);
        }
        LEX_KNOWN_LEN_PUSH(1);
    }
    case ('*'):
    case ('/'):
    case ('%'):
    // bitwise
    case ('|'):
    case ('&'):
    case ('~'):
    case ('^'):
    // boolean
    case ('!'):
        if (n1 == '=') {
            // [sym]=
            LEX_KNOWN_LEN_PUSH(2);
        }
        LEX_KNOWN_LEN_PUSH(1);
    // comparison
    case ('>'): {
        if (pos + 3 < end_of_buf && n1 == '>' && pos[2] == '>' && pos[3] == '=') {
            // >>>=
            LEX_KNOWN_LEN_PUSH(4);
        }
        if (pos + 2 < end_of_buf && n1 == '>' && (pos[2] == '>' || pos[2] == '=')) {
            // >>> or >>=
            LEX_KNOWN_LEN_PUSH(3);
        }
        if (n1 == '>' || n1 == '=') {
            // >> or >=
            LEX_KNOWN_LEN_PUSH(2);
        }
        // >
        LEX_KNOWN_LEN_PUSH(1);
    }
    case ('<'): {
        if (pos + 2 < end_of_buf && n1 == '<' && (pos[2] == '=' || pos[2] == '-')) {
            // <<= or <<-
            LEX_KNOWN_LEN_PUSH(3);
        }
        if (n1 == '<' || n1 == '=' || n1 == '-') {
            // <<, <=, or <-
            LEX_KNOWN_LEN_PUSH(2);
        }
        // // <
        LEX_KNOWN_LEN_PUSH(1);
    }
    case (':'): {
        if (n1 == ':') {
            // ::
            LEX_KNOWN_LEN_PUSH(2);
        }
        // :
        LEX_KNOWN_LEN_PUSH(1);
    }
    default: {
        LOG_ERR("[DEBUG | ERROR] unexpected first character in multichar operator during lexing.");
    }
    }
    goto lex_start;

lex_whitespace:
    if (len != 0) {
        tkn = token_build(start, len, &loc);
        vector_push_back(&tkn_vec, &tkn);
        len = 0;
    }
    ++col;
    loc.col = col;
    ++pos;
    start = pos;
    goto lex_start;

lex_newline:
    ++loc.line;
    loc.col = 0;
    col = 0;
    ++pos;
    ++start;
    goto lex_start;

lex_inline_comment:
    if (len != 0) {
        tkn = token_build(start, len, &loc);
        vector_push_back(&tkn_vec, &tkn);
        len = 0;
        loc.col = col;
        start = pos;
    }
    while (true) {
        c = *pos++;
        if (c == '\n') {
            break;
        }
        if (c == '\0') {
            goto lex_done;
        }
    }
    start = pos;
    len = 0;
    col = 0;
    loc.col = 0;
    ++loc.line;
    goto lex_start;

lex_end:
    if (len != 0) {
        tkn = token_build(start, len, &loc);
        vector_push_back(&tkn_vec, &tkn);
    }
lex_done:
    // build up eof token manually
    tkn.start = start; // shouldn't be read from, but make sure start isn't null just in case
    tkn.length = 1;
    tkn.sym = EOF_TKN;
    tkn.loc = loc; // set loc to currently tracked loc
    vector_push_back(&tkn_vec, &tkn);
    return tkn_vec;
}

void find_lexer_errors(const vector_t* token_vec, compiler_error_list_t* error_list) {
    for (size_t i = 0; i < token_vec->size; i++) {
        token_t* tkn = vector_at(token_vec, i);
        if (tkn->sym == INDETERMINATE) {
            compiler_error_list_emplace(error_list, tkn,
                                        error_message_for(ERR_UNRECOGNIZED_SYMBOL));
        }
    }
}
