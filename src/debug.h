#ifndef DEBUG_H
#define DEBUG_H

#include "lexer.h"
#include "ast.h"

// for debugging purposes
void print_tokens(Token *token);
void print_ast(AstNode *node);
void debug_print_tokens(Lexer *lexer);
void debug_print_ast(AstNode **root, int child_count);

#endif
