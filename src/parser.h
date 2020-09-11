#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

// parser structure
typedef struct Parser {
    Lexer *lexer;
    Token current_token;
} Parser;

// init new parser
Parser parser_init(Lexer *lexer);
// parse tokens into abstract syntax tree
AstNode **parser_parse_prog(Parser *self, int *child_count);

#endif
