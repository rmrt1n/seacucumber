#ifndef AST_H
#define AST_H

#include "lexer.h"

// type for builtin functions
typedef struct AstNode *(*Builtin) (int, struct AstNode **);

// structure of ast node
typedef struct AstNode {
    enum {
        // leaf nodes
        AST_NUMBER, AST_STRING, AST_BOOL,
        AST_NIL, AST_FN, AST_VAR,

        // multiple branches
        AST_BINOP, AST_UNOP, AST_IF,
        AST_ASSIGNMENT, AST_FNCALL, AST_BLOCK,
        AST_CFN,

        AST_NOOP
    } type;

    // binop and unop and assignment
    struct AstNode *left;
    struct AstNode *right;
    Token op;

    // if stmts
    struct AstNode *condition;
    struct AstNode *then_branch;
    struct AstNode *else_branch;

    // function definitions, body for print
    struct AstNode **params;
    int param_count;
    struct AstNode *body;

    // function calls
    struct AstNode **args;
    int arg_count;
    // for anonymous fns
    struct AstNode *lambda;

    // block
    struct AstNode **children;
    int child_count;

    // cfn
    Builtin cfun_ptr;

    // leaf nodes
    Token token;

    // literals
    union {
        char *ident_name;
        double num_value;
        char *str_value;
        int bool_value;
        void *nil;
    } value;
} AstNode;

// functions to create ast node
AstNode *ast_init_num(double num);
AstNode *ast_init_str(char *string);
AstNode *ast_init_bool(int truth);
AstNode *ast_init_nil(void);
AstNode *ast_init_var(char *name, Token token);
AstNode *ast_init_unop(AstNode *right, Token op);
AstNode *ast_init_binop(AstNode *left, AstNode *right, Token op);
AstNode *ast_init_assign(AstNode *left, AstNode *right, Token op);
AstNode *ast_init_if(AstNode *cond, AstNode *then, AstNode *alter);
AstNode *ast_init_block(AstNode **children, int child_count);
AstNode *ast_init_fn(AstNode **params, int param_count, AstNode *body);
AstNode *ast_init_fncall(
    char *fn_name, AstNode **args, int arg_count, AstNode *lambda);
AstNode *ast_init_cfn(char *name, Builtin cfun_ptr);
AstNode *ast_init_noop(void);

#endif
