#ifndef ENV_H
#define ENV_H

#include "ast.h"

// records structure that holds variables as string and
// their value as ast node
struct Records {
    char *varname;
    AstNode *value;
    struct Records *next;
};

// env structure, contains records of the current env, and a reference
// to the parent env
typedef struct Env {
    struct Records *records;
    struct Env *parent;
} Env;

// create an empty env with parent as argument
Env *create_env(Env *parent);
// insert variable and its value to an env
void env_insert_var(Env **env, char *varname, AstNode *value);
// check if a variable is in an env
int env_check_var(Env *env, char *varname);
// return value of a variable
AstNode *env_find_var(Env *env, char *varname);
// insert builtin function to an env
void env_insert_builtin(Env **env, AstNode *cfn);
// uses env_insert_builtin to insert to global env
void env_insert_global_builtin(Env **env);

#endif
