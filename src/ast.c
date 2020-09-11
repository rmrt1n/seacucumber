#include <stdlib.h>
#include "ast.h"

AstNode *ast_init_num(double num) {
    AstNode *node = malloc(sizeof(struct AstNode));

    node->type = AST_NUMBER;
    node->value.num_value = num;

    return node;
}

AstNode *ast_init_str(char *string) {
    AstNode *node = malloc(sizeof(struct AstNode));

    node->type = AST_STRING;
    node->value.str_value = string;

    return node;
}

AstNode *ast_init_bool(int truth) {
    AstNode *node = malloc(sizeof(struct AstNode));

    node->type = AST_BOOL;
    node->value.bool_value = truth == 1;

    return node;
}

AstNode *ast_init_nil(void) {
    AstNode *node = malloc(sizeof(struct AstNode));

    node->type = AST_NIL;
    node->value.nil = NULL;

    return node;
}

AstNode *ast_init_var(char *name, Token token) {
    AstNode *node = malloc(sizeof(struct AstNode));

    node->type = AST_VAR;
    node->token = token;
    node->value.ident_name = name;

    return node;
}

AstNode *ast_init_unop(AstNode *right, Token op) {
    AstNode *node = malloc(sizeof(struct AstNode));
    
    node->type = AST_UNOP;
    node->right = right;
    node->op = op;

    return node;
}

AstNode *ast_init_binop(AstNode *left, AstNode *right, Token op) {
    AstNode *node = malloc(sizeof(struct AstNode));
    
    node->type = AST_BINOP;
    node->left = left;
    node->right = right;
    node->op = op;

    return node;
}

AstNode *ast_init_assign(AstNode *left, AstNode *right, Token op) {
    AstNode *node = malloc(sizeof(struct AstNode));
    
    node->type = AST_ASSIGNMENT;
    node->left = left;
    node->right = right;
    node->op = op;

    return node;
}

AstNode *ast_init_if(AstNode *cond, AstNode *then, AstNode *alter) {
    AstNode *node = malloc(sizeof(struct AstNode));

    node->type = AST_IF;
    node->condition = cond;
    node->then_branch = then;
    node->else_branch = alter;

    return node;
}

AstNode *ast_init_block(AstNode **children, int child_count) {
    AstNode *node = malloc(sizeof(struct AstNode));

    node->type = AST_BLOCK;
    node->children = children;
    node->child_count = child_count;

    return node;
}

AstNode *ast_init_fn(AstNode **params, int param_count, AstNode *body) {
    AstNode *node = malloc(sizeof(struct AstNode));

    node->type = AST_FN;
    node->params = params;
    node->param_count = param_count;
    node->body = body;

    return node;
}

AstNode *ast_init_fncall(
    char *fn_name, AstNode **args, int arg_count, AstNode *lambda) {
    AstNode *node = malloc(sizeof(struct AstNode));

    node->type = AST_FNCALL;
    node->value.ident_name = fn_name;
    node->args = args;
    node->arg_count = arg_count;
    node->lambda = lambda;

    return node;
}

AstNode *ast_init_noop(void) {
    AstNode *node = malloc(sizeof(struct AstNode));
    node->type = AST_NOOP;
    return node;
}

AstNode *ast_init_cfn(char *name, Builtin cfun_ptr) {
    AstNode *node = malloc(sizeof(struct AstNode));

    node->type = AST_CFN;
    node->value.ident_name = name;
    node->cfun_ptr = cfun_ptr;

    return node;
}

