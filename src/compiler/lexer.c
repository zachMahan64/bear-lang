#include "compiler/lexer.h"
#include "containers/vector.h"
#include "token.h"

vector_t lexer_tokenize_src_buffer(const src_buffer_t* src_buffer) {
    vector_t tkn_vec = vector_create_and_reserve(
        sizeof(token_t),
        src_buffer->size / LEXER_ESTIMATED_CHARS_PER_TOKEN); // estimate how large the vector will
                                                             // need to be to minimize reallocs
    // TODO
}
