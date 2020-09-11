#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "lexer.h"
#include "parser.h"
#include "builtin.h"

// main helper funcs
void print_help(void);
char *readfile(char *file_location);
void write(FILE *fp, char *code);

// visitor functions
// instead of executing instructions on tree nodes, return
// ocaml source code to write to file
char *ast_to_str(AstNode *node);
static char *visitor_visit_root(AstNode **root, int child_count);
char *visitor_visit_node(AstNode *node);
char *visitor_visit_assignment(AstNode *node);
char *visitor_visit_var(AstNode *node);
char *visitor_visit_if(AstNode *node);
char *visitor_visit_binop(AstNode *node);
char *visitor_visit_unop(AstNode *node);
char *visitor_visit_block(AstNode *node);
char *visitor_visit_fncall(AstNode *node);
char *visitor_visit_fn(AstNode *node);

int main(int argc, char *argv[]) {
    if (argc == 2) {
        char *contents = readfile(argv[1]);       

        Lexer lexer = lexer_init(contents);
        Parser parser = parser_init(&lexer);

        int child_count = 0;
        AstNode **root = parser_parse_prog(&parser, &child_count);

        char *result = visitor_visit_root(root, child_count);
        FILE *fp = fopen("intermediate.ml", "w");

        write(fp, result);
        system("ocamlc intermediate.ml; rm intermediate*");
    } else {
        print_help();
    }

    return 0;
}

void print_help(void) {
    puts("usage: ./tscc.sh [file]");
}

// read contents of file into a string
char *readfile(char *file_location) {
    char *contents;
    long length = 0;

    FILE *fp = fopen(file_location, "rb");
    if (fp != NULL) {
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        contents = malloc(length + 1);
        if (contents != NULL) fread(contents, 1, length, fp); 
        fclose(fp);

        return contents;
    }
    puts("error reading file");
    exit(1);
}

void write(FILE *fp, char *code) {
    int success = fputs(code, fp);

    if (success == -1) {
        puts("error writing to file");
        exit(1);
    }

    fclose(fp);
}

char *ast_to_str(AstNode *node) {
    char *str;
    switch (node->type) {
        case AST_NUMBER:
            str = malloc(node->value.num_value / 10 + 2);
            if (fmod(node->value.num_value, 1) == 0) {
                sprintf(str, "%d", (int)node->value.num_value);
            } else {
                sprintf(str, "%0.3lf", node->value.num_value);
            }
            break;
        case AST_STRING:
            str = malloc(strlen(node->value.str_value) + 3);
            sprintf(str, "\"%s\"", node->value.str_value);
            break;
        case AST_VAR:
            str = node->value.ident_name;
            break;
        // case AST_UNOP:
            // break;
        case AST_NIL:
        case AST_NOOP:
            str = "nil";
            break;
        case AST_BOOL:
            if (node->value.bool_value == 1) str = "true";
            else str = "false";
            break;
    }

    return str;
}

static char *visitor_visit_root(struct AstNode **root, int child_count) {
    char *result = malloc(1);
    result[0] = '\0';

    for (int i = 0; i < child_count; i++) {
        char *node = visitor_visit_node(root[i]);
        result = realloc(result, strlen(result) + strlen(node) + 5);

        strcat(result, node);
        strcat(result, ";;\n");
        free(node);
    }

    return result;
}

char *visitor_visit_node(AstNode *node) {
    switch (node->type) {
        case AST_NUMBER:
        case AST_STRING:
        case AST_BOOL:
        case AST_NIL:
            return ast_to_str(node);
        case AST_FN:
            return visitor_visit_fn(node);
        case AST_ASSIGNMENT:
            return visitor_visit_assignment(node);
        case AST_VAR:
            return visitor_visit_var(node);
        case AST_IF:
            return visitor_visit_if(node);
        case AST_FNCALL:
            return visitor_visit_fncall(node);
        case AST_UNOP:
            return visitor_visit_unop(node);
        case AST_BLOCK:
            return visitor_visit_block(node);
        case AST_BINOP:
            return visitor_visit_binop(node);
    }
}

char *visitor_visit_assignment(AstNode *node) {
    char *varname = node->left->value.ident_name;
    char *value = visitor_visit_node(node->right);

    char *result = malloc(9);
    result[0] = '\0';
    strcat(result, "let rec ");

    result = realloc(
        result, strlen(result) + strlen(varname) + strlen(value) + 5);

    strcat(result, varname);
    strcat(result, " = ");
    strcat(result, value);

    
    return result;
}

char *visitor_visit_var(AstNode *node) {
    return node->value.ident_name;
}

char *visitor_visit_if(AstNode *node) {
    char *cond = visitor_visit_node(node->condition);
    char *then = visitor_visit_node(node->then_branch);
    char *elsse = visitor_visit_node(node->else_branch);

    char *result = malloc(4);
    result[0] = '\0';
    strcat(result, "if ");
    result = realloc(
        result,
        strlen(result) + strlen(cond) + strlen(then) +
        strlen(elsse) + strlen(" then  else ") + 1
    );
    strcat(result, cond);

    strcat(result, " then ");
    strcat(result, then);

    strcat(result, " else ");
    strcat(result, elsse);

    return result;
}

char *visitor_visit_binop(AstNode *node) {
    char *left = visitor_visit_node(node->left);
    char *right = visitor_visit_node(node->right);

    char *result = malloc(strlen(left) + strlen(right) + 4);
    result[0] = '\0';
    strcat(result, left);

    switch (node->op.type) {
        case TOKEN_PLUS:
            strcat(result, " + ");
            break;
        case TOKEN_MINUS:
            strcat(result, " - ");
            break;
        case TOKEN_MUL:
            strcat(result, " * ");
            break;
        case TOKEN_MOD:
            strcat(result, " % ");
            break;
        case TOKEN_DIV:
            strcat(result, " / ");
            break;
        case TOKEN_LT:
            strcat(result, " < ");
            break;
        case TOKEN_GT:
            strcat(result, " > ");
            break;
        case TOKEN_LTE:
            strcat(result, " <= ");
            break;
        case TOKEN_GTE:
            strcat(result, " >= ");
            break;
        case TOKEN_EQUAL:
            strcat(result, " == ");
            break;
        case TOKEN_NEQUAL:
            strcat(result, " != ");
            break;
        case TOKEN_AND:
            strcat(result, " && ");
            break;
        case TOKEN_OR:
            strcat(result, " || ");
            break;
    }

    strcat(result, right);
    return result;
}

char *visitor_visit_unop(AstNode *node) {
    char *right = visitor_visit_node(node->right);
    char *result = malloc(2);
    result[0] = '\0';

    if (node->op.type == TOKEN_BANG) result[1] = '!';
    else if (node->op.type == TOKEN_MINUS) result[1] = '-';

    result = realloc(result, strlen(result) + strlen(right) + 1);
    strcat(result, right);

    return result; 
}

char *visitor_visit_block(AstNode *node) {
    char *result = malloc(7);
    result[0] = '\0';
    strcat(result, "begin ");
    
    for (int i = 0; i < node->child_count; i++) {
        char *temp = visitor_visit_node(node->children[i]);
        result = realloc(result, strlen(result) + strlen(temp) + 2);

        strcat(result, temp);
        strcat(result, ";");
        free(temp);
    }
    result = realloc(result, strlen(result) + 4);
    strcat(result, "end");
    
    return result;
}


char *visitor_visit_fncall(AstNode *node) {
    char *result = malloc(1);
    result[0] = '\0';

    if (node->value.ident_name != NULL) {
        strcat(result, node->value.ident_name);

        if (node->arg_count == 0) {
            result = realloc(result, strlen(result) + 3);
            strcat(result, "()");
            return result;
        }

        for (int i = 0; i < node->arg_count; i++) {
            char *arg = visitor_visit_node(node->args[i]);

            char *arg_to_cat = malloc(strlen(arg) + 3);
            sprintf(arg_to_cat, "(%s)", arg);

            result = realloc(result, strlen(result) + strlen(arg_to_cat) + 2);

            strcat(result, " ");
            strcat(result, arg_to_cat);

            free(arg);
            free(arg_to_cat);
        }

        return result;
    }

    result = visitor_visit_fn(node->lambda);

    if (node->arg_count == 0) {
        result = realloc(result, strlen(result) + 3);
        strcat(result, "()");
        return result;
    }

    result = realloc(result, strlen(result) + 2);
    strcat(result, " ");

    for (int i = 0; i < node->arg_count; i++) {
        char *arg = visitor_visit_node(node->args[i]);

        char *arg_to_cat = malloc(strlen(arg) + 3);
        sprintf(arg_to_cat, "(%s)", arg);

        result = realloc(result, strlen(result) + strlen(arg_to_cat) + 2);

        strcat(result, " ");
        strcat(result, arg_to_cat);

        free(arg);
        free(arg_to_cat);
    }

    return result;
}

char *visitor_visit_fn(AstNode *node) {
    char *result = malloc(7);
    result[0] = '\0';
    strcat(result, "(fun ");

    for (int i = 0; i < node->param_count; i++) {
        result = realloc(
            result,
            strlen(result) + strlen(node->params[i]->value.ident_name) + 2
        );
        strcat(result, node->params[i]->value.ident_name);
        strcat(result, " ");
    }
    
    char *body = visitor_visit_node(node->body);
    result = realloc(result, strlen(result) + strlen(body) + strlen("-> ") + 2);

    strcat(result, "-> ");
    strcat(result, body);
    strcat(result, ")");
    free(body);

    return result;
}

