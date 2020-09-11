#include <stdlib.h>
#include <string.h>
#include "env.h"
#include "builtin.h"

Env *create_env(Env *parent) {
    Env *env = malloc(sizeof(struct Env));

    env->records = NULL;
    env->parent = parent;
    env->records = NULL;

    return env;
}

void env_insert_var(Env **env, char *varname, AstNode *value) {
    struct Records *record = malloc(sizeof(struct Records));

    record->varname = varname;
    record->value = value;
    record->next = (*env)->records;

    (*env)->records = record;
}

int env_check_var(Env *env, char *varname) {
    struct Env dup = *env;
    struct Env *env_ptr = &dup;

    while (env_ptr != NULL) {
        while (env_ptr->records != NULL) {
            if (strcmp(env_ptr->records->varname, varname) == 0) return 1;
            env_ptr->records = env_ptr->records->next;
        }

        env_ptr = env_ptr->parent;
    }

    return 0;
}

AstNode *env_find_var(Env *env, char *varname) {
    struct Env dup = *env;
    struct Env *env_ptr = &dup;
     
    while (env_ptr != NULL) {
        while (env_ptr->records != NULL) {
            if (strcmp(env_ptr->records->varname, varname) == 0) {
                return env_ptr->records->value;
            }
            env_ptr->records = env_ptr->records->next;
        }

        env_ptr = env_ptr->parent;
    }
}

void env_insert_builtin(Env **env, AstNode *cfn) {
    struct Records *record = malloc(sizeof(struct Records));

    record->varname = cfn->value.ident_name;
    record->value = cfn;
    record->next = (*env)->records;

    (*env)->records = record;
}

// any new builtin function is inserted to global env through this func
void env_insert_global_builtin(Env **env) {
    env_insert_builtin(env, ast_init_cfn("puts", &builtin_puts));
    env_insert_builtin(env, ast_init_cfn("gets", &builtin_gets));
}

