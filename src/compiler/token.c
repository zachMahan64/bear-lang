#include "token.h"
#include "containers/strimap.h"
#include <stdbool.h>

int* get_char_to_token_map(void) {
    static bool initialized = false;
    static int char_to_token_map[128]; // for direct char indexing
    if (!initialized) {
        char_to_token_map['('] = LPAREN;
        char_to_token_map[')'] = RPAREN;
        // TODO finish
        initialized = true;
    }
    return char_to_token_map;
}
