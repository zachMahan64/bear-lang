#include "compiler/lexer.h"
#include "containers/vector.h"
#include "token.h"
#include <stddef.h>

typedef enum { NOT_IN_LIT = 0, IN_CHAR_LIT, IN_STRING_LIT } lexer_delim_state_e;
typedef enum { NOT_IN_ESC = 0, IN_ESCAPE_SEQ = 1 } lexer_esc_seq_state_e;

vector_t lexer_tokenize_src_buffer(const src_buffer_t* src_buffer) {
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
