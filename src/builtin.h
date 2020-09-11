#ifndef BUILTIN_H
#define BUILTIN_H

#include "ast.h"

// puts (print function)
AstNode *builtin_puts(int argc, AstNode **args);
// gets (scanf/fgets)
AstNode *builtin_gets(int argc, AstNode **args);

#endif
