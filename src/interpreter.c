#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "interpreter.h"

static int visitor_seek_truth(AstNode *node);
static AstNode *visitor_visit_node(AstNode *node, Env *env);
static AstNode *visitor_visit_assignment(AstNode *node, Env *env);
static AstNode *visitor_visit_var(AstNode *node, Env *env);
static AstNode *visitor_visit_if(AstNode *node, Env *env);
static AstNode *visitor_visit_binop(AstNode *node, Env *env);
static AstNode *visitor_visit_unop(AstNode *node, Env *env);
static AstNode *visitor_visit_block(AstNode *node, Env *env);
static AstNode *visitor_visit_builtin(AstNode *node, Env *env);
static AstNode *visitor_visit_fncall(AstNode *node, Env *env);

AstNode *visitor_visit_root(struct AstNode **root, int child_count, Env *env) {
    AstNode *node;

    for (int i = 0; i < child_count; i++) {
        node = visitor_visit_node(root[i], env);
    }

    return node;
}

// visit node function
static AstNode *visitor_visit_node(AstNode *node, Env *env) {
    switch (node->type) {
        case AST_NUMBER:
        case AST_STRING:
        case AST_BOOL:
        case AST_NIL:
        case AST_FN:
            return node;
        case AST_ASSIGNMENT:
            return visitor_visit_assignment(node, env);
        case AST_VAR:
            return visitor_visit_var(node, env);
        case AST_IF:
            return visitor_visit_if(node, env);
        case AST_FNCALL:
            return visitor_visit_fncall(node, env);
        case AST_UNOP:
            return visitor_visit_unop(node, env);
        case AST_BLOCK:
            return visitor_visit_block(node, env);
        case AST_BINOP:
            return visitor_visit_binop(node, env);
        default:
            return ast_init_noop();
    }
}

// returns truthy value of expr, everything except nil and false is truthy
static int visitor_seek_truth(AstNode *node) {
    if (node->type == AST_NIL) return 0;
    else if (node->type == AST_BOOL) return node->value.bool_value;
    return 1;
}

// visit ast_assignment, inserts varname and value into env
static AstNode *visitor_visit_assignment(AstNode *node, Env *env) {
    env_insert_var(&env, node->left->value.ident_name, node->right);
    return ast_init_noop();
}

// visit variable, gets variable value from env
static AstNode *visitor_visit_var(AstNode *node, Env *env) {
    if (env_check_var(env, node->value.ident_name) == 0) {
        printf("name \"%s\" is not defined on line %d\n",
               node->value.ident_name, node->token.line);
        exit(1);
    } 
    
    AstNode *var = env_find_var(env, node->value.ident_name);

    if (var->type == AST_FN) {
        var->value.ident_name = node->value.ident_name;
    }

    return visitor_visit_node(var, env);
}

// visit ast_if, get truthy value of condition. if truthy visit then
// branch, else visit else branch
static AstNode *visitor_visit_if(AstNode *node, Env *env) {
    AstNode *cond = visitor_visit_node(node->condition, env);
    int truth = visitor_seek_truth(cond);

    if (truth) {
        return visitor_visit_node(node->then_branch, env);
    }

    return visitor_visit_node(node->else_branch, env);
}

// visit binary node, return new node that is the result of the operation
static AstNode *visitor_visit_binop(AstNode *node, Env *env) {
    AstNode *left = visitor_visit_node(node->left, env);
    AstNode *right = visitor_visit_node(node->right, env);
    AstNode *result;

    switch (node->op.type) {
        case TOKEN_PLUS:
            result = ast_init_num(
                left->value.num_value + right->value.num_value);
            return result;
        case TOKEN_MINUS:
            result = ast_init_num(
                left->value.num_value - right->value.num_value);
            return result;
        case TOKEN_MUL:
            result = ast_init_num(
                left->value.num_value * right->value.num_value);
            return result;
        case TOKEN_MOD:
            result = ast_init_num(
                fmod(left->value.num_value, right->value.num_value));
            return result;
        case TOKEN_DIV:
            result = ast_init_num(
                left->value.num_value / right->value.num_value);
            return result;
        case TOKEN_LT:
            result = ast_init_bool(
                left->value.num_value < right->value.num_value);
            return result;
        case TOKEN_GT:
            result = ast_init_bool(
                left->value.num_value > right->value.num_value);
            return result;
        case TOKEN_LTE:
            result = ast_init_bool(
                left->value.num_value <= right->value.num_value);
            return result;
        case TOKEN_GTE:
            result = ast_init_bool(
                left->value.num_value >= right->value.num_value);
            return result;
        case TOKEN_EQUAL:
            result = ast_init_bool(
                left->value.num_value == right->value.num_value);
            return result;
        case TOKEN_NEQUAL:
            result = ast_init_bool(
                left->value.num_value != right->value.num_value);
            return result;
        case TOKEN_AND:
            result = ast_init_bool(
                left->value.num_value && right->value.num_value);
            return result;
        case TOKEN_OR:
            result = ast_init_bool(
                left->value.num_value || right->value.num_value);
            return result;
    }
}

// visit unary node, return new node with value of operation
static AstNode *visitor_visit_unop(AstNode *node, Env *env) {
    AstNode *result = visitor_visit_node(node->right, env);

    if (node->op.type == TOKEN_BANG) {
        result->value.bool_value = result->value.bool_value == 1 ? 0 : 1;
    } else if (node->op.type == TOKEN_MINUS) {
        result->value.num_value *= -1;
    }

    return result; 
}

// visit block statement (still doesn't work)
static AstNode *visitor_visit_block(AstNode *node, Env *env) {
    AstNode *expr;
    Env *local_env = create_env(env);
    
    for (int i = 0; i < node->child_count; i++) {
        expr = visitor_visit_node(node->children[i], local_env);
    }
    
    return expr;
}

// visit builtin function(c function pointer), get function pointer
// and call it with args
static AstNode *visitor_visit_builtin(AstNode *node, Env *env) {
    AstNode *cfn = env_find_var(env, node->value.ident_name);

    AstNode **evaled_args = malloc(node->arg_count * sizeof(struct AstNode *));
    for (int i = 0; i < node->arg_count; i++) {
        evaled_args[i] = visitor_visit_node(node->args[i], env);
    }

    return cfn->cfun_ptr(node->arg_count, evaled_args);
}

// visit fncall. for named functions :
// visit func call get the node from env, create local env for function
// and assign arg values to params, and call visit_node with local env.
// else if is anonymous function, do the same without getting value from
// env
static AstNode *visitor_visit_fncall(AstNode *node, Env *env) {
    // check if function exists
    if (node->value.ident_name != NULL) {
        if (env_check_var(env, node->value.ident_name) == 0) {
            printf("func \"%s\" is not defined on line %d\n",
                   node->value.ident_name, node->token.line);
            exit(1);
        }

        // get fn ast node
        AstNode *fn = env_find_var(env, node->value.ident_name);

        // if is builtin function
        if (fn->type == AST_CFN) return visitor_visit_builtin(node, env);

        // check if args count is same as params count
        if (node->arg_count != fn->param_count) {
            printf(
                "invalid number of arguments. fn takes %d args, %d given\n",
                fn->param_count, node->arg_count);
            exit(1);
        }

        // create local scope of function
        Env *local_env = create_env(env);

        // insert into local env params with values of args
        for (int i = 0; i < node->arg_count; i++) {
            AstNode *arg = visitor_visit_node(node->args[i], env);
            env_insert_var(&local_env, fn->params[i]->value.ident_name, arg);
        }

        return visitor_visit_node(fn->body, local_env);
    }

    if (node->arg_count != node->lambda->param_count) {
        printf(
            "invalid number of arguments. fn takes %d args, %d given\n",
            node->lambda->param_count, node->arg_count);
        exit(1);
    }

    Env *local_env = create_env(env);

    for (int i = 0; i < node->arg_count; i++) {
        AstNode *arg = visitor_visit_node(node->args[i], env);
        env_insert_var(
            &local_env, node->lambda->params[i]->value.ident_name, arg);
    }

    return visitor_visit_node(node->lambda->body, local_env);
}

