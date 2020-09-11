#include <stdio.h>
#include "debug.h"

// token to string function
void print_tokens(Token *token) {
    char *type;
    switch (token->type) {
        case TOKEN_NUMBER: type = "NUMBER"; break;
        case TOKEN_STRING: type = "STRING"; break;
        case TOKEN_IDENT:  type = "IDENT"; break;
        case TOKEN_TRUE:   type = "TRUE"; break;
        case TOKEN_FALSE:  type = "FALSE"; break;
        case TOKEN_NIL:    type = "NIL"; break;
        case TOKEN_LET:    type = "LET"; break;
        case TOKEN_IF:     type = "IF"; break;
        case TOKEN_THEN:   type = "THEN"; break;
        case TOKEN_ELSE:   type = "ELSE"; break;
        case TOKEN_FN:     type = "FN"; break;
        case TOKEN_AND:    type = "AND"; break;
        case TOKEN_OR:     type = "OR"; break;
        case TOKEN_RPAREN: type = "RPAREN"; break;
        case TOKEN_LPAREN: type = "LPAREN"; break;
        case TOKEN_LT:     type = "LT"; break;
        case TOKEN_LTE:    type = "LTE"; break;
        case TOKEN_GT:     type = "GT"; break;
        case TOKEN_GTE:    type = "GTE"; break;
        case TOKEN_SEMI:   type = "SEMI"; break;
        case TOKEN_COMMA:  type = "COMMA"; break;
        case TOKEN_ASSIGN: type = "ASSIGN"; break;
        case TOKEN_EQUAL:  type = "EQUAL"; break;
        case TOKEN_NEQUAL: type = "NEQUAL"; break;
        case TOKEN_BANG:   type = "BANG"; break;
        case TOKEN_ARROW:  type = "ARROW"; break;
        case TOKEN_PLUS:   type = "PLUS"; break;
        case TOKEN_MINUS:  type = "MINUS"; break;
        case TOKEN_MUL:    type = "MUL"; break;
        case TOKEN_DIV:    type = "DIV"; break;
        case TOKEN_MOD:    type = "MOD"; break;
        case TOKEN_DO:     type = "DO"; break;
        case TOKEN_DONE:   type = "DONE"; break;
        case TOKEN_EOF:    type = "EOF"; break;
    }

    printf("TOKEN[type: %s, value: \"%s\", line: %d]\n",
            type, token->value, token->line);
}

// hacky ast to string function
void print_ast(AstNode *node) {
    switch (node->type) {
        case AST_NUMBER:
            printf("NUMBER ");
            printf("%f\n", node->value.num_value);
            break;
        case AST_STRING:
            printf("STRING ");
            printf("%s\n", node->value.str_value);
            break;
        case AST_VAR:
            printf("VAR ");
            printf("%s\n", node->value.ident_name);
            break;
        case AST_UNOP:
            printf("UNOP\n");
            printf("operator %s\n", node->op.value);
            printf("right ");
            print_ast(node->right);
            break;
        case AST_BINOP:
            printf("BINOP\n");
            printf("left ");
            print_ast(node->left);
            printf("operator %s\n", node->op.value);
            printf("right ");
            print_ast(node->right);
            break;
        case AST_IF:
            printf("IF\n");
            printf("cond ");
            print_ast(node->condition);
            printf("then ");
            print_ast(node->then_branch);
            printf("else ");
            print_ast(node->else_branch);
            break;
        case AST_FN:
            printf("FN\n");
            printf("params ");
            for (int i = 0; i < node->param_count; i++) {
                print_ast(node->params[i]);
            }
            printf("body ");
            print_ast(node->body);
            break;
        case AST_ASSIGNMENT:
            printf("ASS\n");
            printf("varname ");
            print_ast(node->left);
            printf("value ");
            print_ast(node->right);
            break;
        case AST_FNCALL:
            printf("FNCALL\n");
            printf("fn name %s", node->value.ident_name);
            puts("");
            printf("args ");
            for (int i = 0; i < node->arg_count; i++) {
                print_ast(node->args[i]);
            }
            break;
        case AST_NIL:
        case AST_NOOP:
            puts("nil");
            break;
        case AST_BOOL:
            if (node->value.bool_value == 1) {
                puts("true");
            } else {
                puts("false");
            }
            break;
        case AST_BLOCK:
            puts("DO");
            for (int i = 0; i < node->child_count; i++) {
                print_ast(node->children[i]);
            }
            puts("DONE");
    }
}

// run print tokens on lexer
void debug_print_tokens(Lexer *lexer) {
    Token token;
    do {
        token = lexer_get_next_token(lexer);
        print_tokens(&token);
    } while (token.type != TOKEN_EOF);
}

// run print ast on root node
void debug_print_ast(AstNode **root, int child_count) {
    printf("stmt count: %d\n", child_count);
    for (int i = 0; i < child_count; i++) {
        print_ast(root[i]);
    }
}

