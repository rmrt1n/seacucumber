#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "env.h"

// visitor every node in root node
AstNode *visitor_visit_root(AstNode **root, int child_count, Env *env);

#endif
