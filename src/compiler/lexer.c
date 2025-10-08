#include "compiler/lexer.h"
#include "containers/vector.h"
#include "log.h"
#include "token.h"
#include <stddef.h>

typedef enum { NOT_IN_LIT = 0, IN_CHAR_LIT, IN_STRING_LIT } lexer_delim_state_e;
typedef enum { NOT_IN_ESC = 0, IN_ESCAPE_SEQ = 1 } lexer_esc_seq_state_e;

vector_t lexer_naively_by_whitespace_tokenize_src_buffer(const src_buffer_t* src_buffer) {
    vector_t tkn_vec = vector_create_and_reserve(
        sizeof(token_t), src_buffer->size / LEXER_ESTIMATED_CHARS_PER_TOKEN);

    size_t start = 0;
    size_t len = 0;

    lexer_delim_state_e delim_state = NOT_IN_LIT;
    lexer_esc_seq_state_e esc_seq_state = NOT_IN_ESC;

    token_t tkn;
    src_loc_t loc = {.line = 0, .col = 0};         // location of *pos*
    src_loc_t token_start = {.line = 0, .col = 0}; // location of token start

    const char* data = src_buffer->data;
    char* pos = (char*)data;

    while (*pos) {
        char c = *pos;

        // newline: always treat as delimiter
        if (c == '\n') {
            if (len > 0) {
                tkn = token_build(data + start, len, &token_start);
                vector_push_back(&tkn_vec, &tkn);
                start = (pos - data) + 1;
                len = 0;
            }
            ++pos;
            ++loc.line;
            loc.col = 0; // next char is column 0
            delim_state = NOT_IN_LIT;
            continue;
        }

        // whitespace delimiter (outside literals)
        if ((c == ' ' || c == '\t') && delim_state == NOT_IN_LIT) {
            if (len > 0) {
                tkn = token_build(data + start, len, &token_start);
                vector_push_back(&tkn_vec, &tkn);
                start = (pos - data) + 1;
                len = 0;
            }
            ++pos;
            ++loc.col;
            continue;
        }

        // handle escape start (only enter; reset after consuming next char)
        if (c == '\\') {
            if (len == 0) {
                token_start = loc;
            }
            ++len;
            ++pos;
            ++loc.col;
            esc_seq_state = IN_ESCAPE_SEQ;
            // consume next char as part of token if present
            if (*pos) {
                ++len;
                ++pos;
                ++loc.col;
            }
            esc_seq_state = NOT_IN_ESC;
            continue;
        }

        // entering literals
        if (c == '\'' && delim_state == NOT_IN_LIT) {
            delim_state = IN_CHAR_LIT;
        } else if (c == '"' && delim_state == NOT_IN_LIT) {
            delim_state = IN_STRING_LIT;
        }

        // exiting literals (only when not escaped)
        if ((delim_state == IN_CHAR_LIT && c == '\'') ||
            (delim_state == IN_STRING_LIT && c == '"')) {
            if (esc_seq_state == NOT_IN_ESC) {
                ++len; // include the closing quote
                ++pos;
                ++loc.col;
                delim_state = NOT_IN_LIT;
                tkn = token_build(data + start, len, &token_start);
                vector_push_back(&tkn_vec, &tkn);
                start = (pos - data);
                len = 0;
                continue;
            }
            // if it's escaped, fallthrough and consume normally (escape was handled earlier)
        }

        // normal accumulation
        if (len == 0) {
            token_start = loc; // token starts at current loc
        }
        ++len;
        ++pos;
        ++loc.col;
    }

    // trailing token (if any)
    if (len > 0) {
        tkn = token_build(data + start, len, &token_start);
        vector_push_back(&tkn_vec, &tkn);
    }

    return tkn_vec;
}

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
    char c;

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
    if (c == '\0') {
        goto lex_end;
    }
    ++pos;
    ++len;
    ++col;
    goto lex_start;

lex_multichar_operator:
    switch (c) {
    case ('.'): {
        if (pos + 1 < end_of_buf && pos[1] == '.') {
            LEX_KNOWN_LEN_PUSH(2);
        }
        LEX_KNOWN_LEN_PUSH(1);
    }
    // assignment
    case ('='): {
        if (pos + 1 < end_of_buf && pos[1] == '=') {
            LEX_KNOWN_LEN_PUSH(2);
        }
        LEX_KNOWN_LEN_PUSH(1);
    }

    // arithmetic
    case ('+'): {
        if (pos + 1 < end_of_buf && (pos[1] == '=' || pos[1] == '+')) {
            // ++ or +=
            LEX_KNOWN_LEN_PUSH(2);
        }
        LEX_KNOWN_LEN_PUSH(1);
    }
    case ('-'): {
        if (pos + 1 < end_of_buf && (pos[1] == '=' || pos[1] == '-' || pos[1] == '>')) {
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
        if (pos + 1 < end_of_buf && pos[1] == '=') {
            // [sym]=
            LEX_KNOWN_LEN_PUSH(2);
        }
        LEX_KNOWN_LEN_PUSH(1);
    // comparison
    case ('>'): {
        if (pos + 3 < end_of_buf && pos[1] == '>' && pos[2] == '>' && pos[3] == '=') {
            // >>>=
            LEX_KNOWN_LEN_PUSH(2);
        }
        if (pos + 2 < end_of_buf && pos[1] == '>' && (pos[2] == '>' || pos[2] == '=')) {
            // >>> or >>=
            LEX_KNOWN_LEN_PUSH(2);
        }
        if (pos + 1 < end_of_buf && (pos[1] == '>' || pos[1] == '=')) {
            // >> or >=
            LEX_KNOWN_LEN_PUSH(2);
        }
        // >
        LEX_KNOWN_LEN_PUSH(1);
    }
    case ('<'): {
        if (pos + 2 < end_of_buf && pos[1] == '<' && pos[2] == '=') {
            // <<=
            LEX_KNOWN_LEN_PUSH(2);
        }
        if (pos + 1 < end_of_buf && (pos[1] == '<' || pos[1] == '=' || pos[1] == '-')) {
            // <<, <=, or <-
            LEX_KNOWN_LEN_PUSH(2);
        }
        // // <
        LEX_KNOWN_LEN_PUSH(1);
    }
    case (':'): {
        if (pos + 1 < end_of_buf && pos[1] == ':') {
            // ::
            LEX_KNOWN_LEN_PUSH(2);
        }
        LEX_KNOWN_LEN_PUSH(1);
    }
    default: {
        LOG_ERR("[DEBUG | ERROR] unexpect first character in multichar operator during lexing.");
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
    return tkn_vec;
}
