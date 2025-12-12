// primary function for parsing a file into an ast
#include "compiler/ast/stmt.h"
#include "compiler/parser/parser.h"
#include "compiler/parser/token_eaters.h"
#include "utils/vector.h"

size_t parser_estimate_stmt_cnt(vector_t* tkn_vec) {
    // num stmts roughly <= num lines
    token_t* tkn = vector_last(tkn_vec);
    return tkn->loc.line + 1; // since zero-indexed
}

ast_stmt_file_t parser_file(parser_t parser, const char* file_name, vector_t token_vec) {

    ast_stmt_file_t file = {
        .stmt_vec =
            vector_create_and_reserve(sizeof(ast_stmt_t), parser_estimate_stmt_cnt(&token_vec)),
        .file_name = file_name,
    };

    // TODO, AST building up logic here
    // right now this is just some placeholder logic to test the basic parser functions
    // no ast is being raised, we're just running through the tokens
    token_t* tkn = NULL; // scratch token
    while (!parser_eof(&parser)) {
        tkn = parser_match_token(&parser, TOK_INDETERMINATE);
        if (tkn) {
            compiler_error_list_emplace(parser.error_list, tkn, ERR_ILLEGAL_IDENTIFER);
        }
        // expect id or other after builtin type
        else if (parser_match_token_call(&parser, &parser_match_is_builtin_type)) {
            tkn = parser_match_token(&parser, TOK_IDENTIFIER);
            if (!tkn) {
                tkn = parser_match_token(&parser, TOK_SELF_ID);
            }
            if (!tkn) {
                tkn = parser_match_token(&parser, TOK_MUT);
            }
            if (!tkn) {
                tkn = parser_match_token(&parser, TOK_STAR);
            }
            if (!tkn) {
                tkn = parser_match_token(&parser, TOK_AMPER);
            }
            if (!tkn) {
                tkn = parser_match_token(&parser, TOK_LBRACK);
            }

            if (!tkn) {
                compiler_error_list_emplace(parser.error_list, tkn = parser_eat(&parser),
                                            ERR_EXPECTED_IDENTIFIER);
            }

        }

        // expect type after rarrow
        else if (parser_match_token(&parser, TOK_RARROW)) {
            parser_expect_token_call(&parser, &parser_match_is_builtin_type_or_id,
                                     ERR_EXPECTED_TYPE);
        }

        // just consume
        else {
            parser_eat(&parser);
        }
    }
    return file;
}
