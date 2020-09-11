#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "env.h"
#include "builtin.h"

// main helper funcs
void readline(char **line);
void repl(Env *env);
void print_help(void);
char *readfile(char *file_location);

int main(int argc, char *argv[]) {
    Env *global_env = create_env(NULL);
    env_insert_global_builtin(&global_env);

    if (argc == 2) {
        char *contents = readfile(argv[1]);       

        Lexer lexer = lexer_init(contents);
        // debug_print_tokens(&lexer);
        Parser parser = parser_init(&lexer);

        int child_count = 0;
        AstNode **root = parser_parse_prog(&parser, &child_count);

        // debug_print_ast(root, child_count);
        visitor_visit_root(root, child_count, global_env);
    } else if (argc == 1) {
        repl(global_env);
    } else {
        print_help();
    }

    return 0;
}

// get input from stdin
void readline(char **line) {
    size_t size = 0;
    if (getline(line, &size, stdin) == -1) {
        puts("error reading input");
        exit(1);
    }
}

void repl(Env *env) {
    char *line = NULL;
    while (1) {
        printf("|> ");
        readline(&line);

        if (strcmp(line, "quit\n") == 0) exit(0);

        Lexer lexer = lexer_init(line);
        // debug_print_tokens(&lexer);

        Parser parser = parser_init(&lexer);

        int child_count = 0;
        AstNode **root = parser_parse_prog(&parser, &child_count);
        // debug_print_ast(root, child_count);

        AstNode *result = visitor_visit_root(root, child_count, env);
        builtin_puts(child_count, &result);
    }
}

void print_help(void) {
    puts("usage: scc [file]");
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

