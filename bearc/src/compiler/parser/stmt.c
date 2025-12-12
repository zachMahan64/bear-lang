// primary function for parsing a file into an ast
#include "compiler/ast/stmt.h"
#include "compiler/parser/parser.h"
#include "compiler/parser/stmt.h"
#include "compiler/parser/token_eaters.h"
#include "utils/arena.h"
#include "utils/vector.h"
#include <string.h>

size_t parser_estimate_stmt_cnt(vector_t* tkn_vec) {
    // num stmts roughly <= num lines
    token_t* tkn = vector_last(tkn_vec);
    return tkn->loc.line + 1; // since zero-indexed
}

ast_stmt_file_t parser_file(parser_t* p, const char* file_name) {
    vector_t stmt_vec =
        vector_create_and_reserve(sizeof(ast_stmt_t), parser_estimate_stmt_cnt(p->tokens));

    // TODO, AST building up logic here
    // right now this is just some placeholder logic to test the basic parser functions
    // no ast is being raised, we're just running through the tokens
    token_t* tkn = NULL; // scratch token
    while (!parser_eof(p)) {
        tkn = parser_match_token(p, TOK_INDETERMINATE);
        if (tkn) {
            compiler_error_list_emplace(p->error_list, tkn, ERR_ILLEGAL_IDENTIFER);
        }
        // expect id or other after builtin type
        else if (parser_match_token_call(p, &parser_match_is_builtin_type)) {
            tkn = parser_match_token(p, TOK_IDENTIFIER);
            if (!tkn) {
                tkn = parser_match_token(p, TOK_SELF_ID);
            }
            if (!tkn) {
                tkn = parser_match_token(p, TOK_MUT);
            }
            if (!tkn) {
                tkn = parser_match_token(p, TOK_STAR);
            }
            if (!tkn) {
                tkn = parser_match_token(p, TOK_AMPER);
            }
            if (!tkn) {
                tkn = parser_match_token(p, TOK_LBRACK);
            }

            if (!tkn) {
                compiler_error_list_emplace(p->error_list, tkn = parser_eat(p),
                                            ERR_EXPECTED_IDENTIFIER);
            }

        }

        // expect type after rarrow
        else if (parser_match_token(p, TOK_RARROW)) {
            parser_expect_token_call(p, &parser_match_is_builtin_type_or_id, ERR_EXPECTED_TYPE);
        }

        // just consume
        else {
            parser_eat(p);
        }
    }
    ast_stmt_file_t file = {
        .file_name = file_name,
        .stmts = parser_freeze_stmt_vec(p, &stmt_vec),
    };
    return file;
}

ast_slice_of_stmts_t parser_freeze_stmt_vec(parser_t* p, vector_t* vec) {
    ast_slice_of_stmts_t slice = {
        .start = arena_alloc(p->arena, vec->size * vec->elem_size),
        .len = vec->size,
    };
    memcpy(slice.start, vec->data, vec->size * vec->elem_size);
    vector_destroy(vec);
    return slice;
}
