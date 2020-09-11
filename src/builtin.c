#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "builtin.h"

AstNode *builtin_puts(int argc, AstNode **args) {
    for (int i = 0; i < argc; i++) {
        switch (args[i]->type) {
            case AST_NUMBER:
                if (fmod(args[i]->value.num_value, 1) == 0) {
                    printf("%d", (int)args[i]->value.num_value);
                } else {
                    printf("%lf", args[i]->value.num_value);
                }
                break;
            case AST_STRING:
                printf("%s", args[i]->value.str_value);
                break;
            case AST_BOOL:
                if (args[i]->value.bool_value == 1) {
                    printf("true");
                } else {
                    printf("false");
                }
                break;
            case AST_NIL:
                printf("nil");
                break;
            case AST_FN:
                if (args[i]->value.ident_name == NULL) {
                    printf("<lambda expression>");
                } else {
                    printf("<function %s>", args[i]->value.ident_name);
                }
                break;
            default:
                printf("");
                return ast_init_noop();
        }
    }
    printf("\n");
    return ast_init_noop();
}

AstNode *builtin_gets(int argc, AstNode **args) {
    if (argc > 1) {
        printf("gets expect at most 1 argument, got %d\n", argc);
        exit(1);
    } else if (argc == 1) {
        if (args[0]->type != AST_STRING) {
            puts("gets only takes string as an argument");
            exit(1);
        }
        printf("%s", args[0]->value.str_value);
    }

    char *result = NULL;
    size_t size = 0;
    ssize_t len = getline(&result, &size, stdin);

    if (len == -1) {
        puts("error reading input");
        exit(1);
    }

    result[len - 1] = '\0';
    return ast_init_str(result);
}

