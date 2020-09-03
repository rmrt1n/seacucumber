#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>


typedef struct Token {
    enum {
        // literals and identifier
        TOKEN_NUMBER, TOKEN_STRING, TOKEN_IDENT,

        // keywords
        TOKEN_LET, TOKEN_AND, TOKEN_OR,
        TOKEN_IF, TOKEN_THEN, TOKEN_ELSE,
        TOKEN_PUTS, TOKEN_GETS, TOKEN_DO, TOKEN_DONE,
        TOKEN_FN, TOKEN_TRUE, TOKEN_FALSE, TOKEN_NIL,

        // symbols and operators
        TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_SEMI,
        TOKEN_ASSIGN, TOKEN_EQUAL, TOKEN_NEQUAL,
        TOKEN_LT, TOKEN_LTE, TOKEN_GT, TOKEN_GTE,
        TOKEN_ARROW, TOKEN_MOD, TOKEN_COMMA, TOKEN_BANG,
        TOKEN_PLUS, TOKEN_MINUS, TOKEN_MUL, TOKEN_DIV,

        TOKEN_EOF
    } type;
    char *value;
    int line;
} Token;

typedef struct Lexer {
    char *contents;
    int pos;
    char current_char;
    int line;
} Lexer;

typedef struct AstNode {
    enum {
        // leaf nodes
        AST_NUMBER, AST_STRING, AST_BOOL,
        AST_NIL, AST_FN, AST_VAR,

        // multiple branches
        AST_BINOP, AST_UNOP, AST_IF,
        AST_ASSIGNMENT, AST_FNCALL, AST_BLOCK,

        // io
        AST_PUTS, AST_GETS,

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
    // struct AstNode *fn_name;
    struct AstNode **args;
    int arg_count;
    // for anonymous fns
    struct AstNode *lambda;

    // block
    struct AstNode **children;
    int child_count;

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

typedef struct Parser {
    Lexer *lexer;
    Token current_token;
} Parser;

struct Records {
    char *varname;
    AstNode *value;
    struct Records *next;
};

typedef struct Env {
    struct Records *records;
    struct Env *parent;
} Env;


// lexer functions
Token create_token(int token_type, char *value, int line);
Lexer lexer_init(char *contents);
void lexer_advance(Lexer *self);
void lexer_skip_whitespace(Lexer *self);
void lexer_ignore_comments(Lexer *self);
Token lexer_get_next_token(Lexer *self);

// ast node functions
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
AstNode *ast_init_puts(AstNode *body);
AstNode *ast_init_noop(void);

// parser functions
Parser parser_init(Lexer *lexer);
void parser_eat(Parser *self, int token_type);
AstNode **parser_parse_prog(Parser *self, int *child_count);
AstNode *parser_parse_form(Parser *self);
AstNode *parser_parse_assignment(Parser *self);
AstNode *parser_parse_expr(Parser *self);
AstNode *parser_parse_logical_or(Parser *self);
AstNode *parser_parse_logical_and(Parser *self);
AstNode *parser_parse_equality(Parser *self);
AstNode *parser_parse_comparison(Parser *self);
AstNode *parser_parse_addition(Parser *self);
AstNode *parser_parse_multiplication(Parser *self);
AstNode *parser_parse_unary(Parser *self);
AstNode *parser_parse_call(Parser *self);
AstNode *parser_parse_primary(Parser *self);

// env functions
Env *create_env(Env *parent);
void env_insert(Env **env, char *varname, AstNode *value);
int env_check_var(Env *env, char *varname);
AstNode *env_find_var(Env *env, char *varname);

// builtin functions
void builtin_puts(AstNode *node);
char *builtin_gets(void);

// visitor functions
AstNode *visitor_visit_root(AstNode **root, int child_count, Env *env);
AstNode *visitor_visit_node(AstNode *node, Env *env);
int visitor_seek_truth(AstNode *node);
AstNode *visitor_visit_assignment(AstNode *node, Env *env);
AstNode *visitor_visit_var(AstNode *node, Env *env);
AstNode *visitor_visit_if(AstNode *node, Env *env);
AstNode *visitor_visit_binop(AstNode *node, Env *env);
AstNode *visitor_visit_unop(AstNode *node, Env *env);
AstNode *visitor_visit_block(AstNode *node, Env *env);
AstNode *visitor_visit_fncall(AstNode *node, Env *env);

// main helper funcs
void readline(char **line);
void repl(Env *env);
void print_help(void);
char *readfile(char *file_location);
void print_tokens(Token *token);
void print_ast(AstNode *node);

// for debugging purposes
void debug_print_tokens(Lexer *lexer);
void debug_print_ast(AstNode **root, int child_count);


int main(int argc, char *argv[]) {
    Env *global_env = create_env(NULL);

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
        builtin_puts(result);
    }
}

void print_help(void) {
    puts("usage: scc [file]");
}

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
        case TOKEN_PUTS:   type = "PUTS"; break;
        case TOKEN_DO:     type = "DO"; break;
        case TOKEN_DONE:   type = "DONE"; break;
        case TOKEN_EOF:    type = "EOF"; break;
    }

    printf("TOKEN[type: %s, value: \"%s\", line: %d]\n",
            type, token->value, token->line);
}

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
        case AST_PUTS:
            printf("PUTS\n");
            printf("to print ");
            print_ast(node->body);
            break;
        case AST_FNCALL:
            printf("FNCALL\n");
            printf("fn name ", node->value.ident_name);
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

Token create_token(int token_type, char *value, int line) {
    Token token = {token_type, value, line};
    return token;
}

Lexer lexer_init(char *contents) {
    Lexer lexer;

    lexer.contents = contents;
    lexer.pos = 0;
    lexer.current_char = contents[0];
    lexer.line = 1;

    return lexer;
}

void lexer_advance(Lexer *self) {
    if (self->pos < strlen(self->contents)) {
        if (self->current_char == '\n') self->line++;

        self->pos++;
        self->current_char = self->contents[self->pos];
    }
}

void lexer_skip_whitespace(Lexer *self) {
    while (isspace(self->current_char)) {
        lexer_advance(self);
    }
}

void lexer_ignore_comments(Lexer *self) {
    while (self->current_char != '\n') lexer_advance(self);
    lexer_advance(self);
}

Token lexer_get_next_token(Lexer *self) {
    if (self->pos < strlen(self->contents)) {
        // skip whitespace
        if (isspace(self->current_char)) lexer_skip_whitespace(self);

        // ignore comments
        if (self->current_char == '#') lexer_ignore_comments(self);

        // get identifiers and keywods
        if (isalpha(self->current_char)) {
            char *ident = malloc(1);
            ident[0] = '\0';

            while (isalnum(self->current_char) || self->current_char == '_') {
                char *temp = malloc(2);
                temp[0] = self->current_char; temp[1] = '\0';

                ident = realloc(ident, (strlen(ident) + 2));
                strcat(ident, temp);
                free(temp);

                lexer_advance(self);
            }

            int token_type = TOKEN_IDENT;

            if (strcmp(ident, "let") == 0) {
                token_type = TOKEN_LET;
            } else if (strcmp(ident, "if") == 0) {
                token_type = TOKEN_IF;
            } else if (strcmp(ident, "then") == 0) {
                token_type = TOKEN_THEN;
            } else if (strcmp(ident, "else") == 0) {
                token_type = TOKEN_ELSE;
            } else if (strcmp(ident, "fn") == 0) {
                token_type = TOKEN_FN;
            } else if (strcmp(ident, "true") == 0) {
                token_type = TOKEN_TRUE;
            } else if (strcmp(ident, "false") == 0) {
                token_type = TOKEN_FALSE;
            } else if (strcmp(ident, "nil") == 0) {
                token_type = TOKEN_NIL;
            } else if (strcmp(ident, "and") == 0) {
                token_type = TOKEN_AND;
            } else if (strcmp(ident, "or") == 0) {
                token_type = TOKEN_OR;
            } else if (strcmp(ident, "puts") == 0) {
                token_type = TOKEN_PUTS;
            } else if (strcmp(ident, "gets") == 0) {
                token_type = TOKEN_GETS;
            } else if (strcmp(ident, "do") == 0) {
                token_type = TOKEN_DO;
            } else if (strcmp(ident, "done") == 0) {
                token_type = TOKEN_DONE;
            }

            return create_token(token_type, ident, self->line);
        }

        if (isdigit(self->current_char)) {
            char *number = malloc(1);
            number[0] = '\0';

            while (isdigit(self->current_char) || self->current_char == '.') {
                char *temp = malloc(2);
                temp[0] = self->current_char; temp[1] = '\0';
                
                number = realloc(number, (strlen(number) + 2));
                strcat(number, temp);
                free(temp);

                lexer_advance(self);
            }

            return create_token(TOKEN_NUMBER, number, self->line);
        }

        switch (self->current_char) {
            case '(':
                lexer_advance(self);
                return create_token(TOKEN_LPAREN, "(", self->line);
            case ')':
                lexer_advance(self);
                return create_token(TOKEN_RPAREN, ")", self->line);
            case '+':
                lexer_advance(self);
                return create_token(TOKEN_PLUS, "+", self->line);
            case '*': 
                lexer_advance(self);
                return create_token(TOKEN_MUL, "*", self->line);
            case '/': 
                lexer_advance(self);
                return create_token(TOKEN_DIV, "/", self->line);
            case '%': 
                lexer_advance(self);
                return create_token(TOKEN_MOD, "%", self->line);
            case ';': 
                lexer_advance(self);
                return create_token(TOKEN_SEMI, ";", self->line);
            case ',': 
                lexer_advance(self);
                return create_token(TOKEN_COMMA, ",", self->line);
            case '-':
                lexer_advance(self);
                if (self->current_char == '>') {
                    lexer_advance(self);
                    return create_token(TOKEN_ARROW, "->", self->line);
                }
                return create_token(TOKEN_MINUS, "-", self->line);
            case '=':
                lexer_advance(self);
                if (self->current_char == '=') {
                    lexer_advance(self);
                    return create_token(TOKEN_EQUAL, "==", self->line);
                }
                return create_token(TOKEN_ASSIGN, "=", self->line);
            case '!':
                lexer_advance(self);
                if (self->current_char == '=') {
                    lexer_advance(self);
                    return create_token(TOKEN_NEQUAL, "!=", self->line);
                }
                return create_token(TOKEN_BANG, "!", self->line);
            case '<':
                lexer_advance(self);
                if (self->current_char == '=') {
                    lexer_advance(self);
                    return create_token(TOKEN_LTE, "<=", self->line);
                }
                return create_token(TOKEN_LT, "<", self->line);
            case '>':
                lexer_advance(self);
                if (self->current_char == '=') {
                    lexer_advance(self);
                    return create_token(TOKEN_GTE, ">=", self->line);
                }
                return create_token(TOKEN_GT, ">", self->line);
            case '\"':
                lexer_advance(self);
                char *str = malloc(1);
                str[0] = '\0';

                while (self->current_char != '\"') {
                    char *temp = malloc(2);
                    temp[0] = self->current_char; temp[1] = '\0';

                    str = realloc(str, (strlen(str) + 2));
                    strcat(str, temp);
                    free(temp);

                    lexer_advance(self);
                }

                lexer_advance(self);
                return create_token(TOKEN_STRING, str, self->line);
            // as a result of skipping ws
            case '\0': break;
            default:
                printf("error: unexpected character '%c' at line %d\n",
                        self->current_char, self->line);
        }       
    }

    return create_token(TOKEN_EOF, "\0", self->line - 1);
}

// ast functions
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

AstNode *ast_init_puts(AstNode *body) {
    AstNode *node = malloc(sizeof(struct AstNode));

    node->type = AST_PUTS;
    node->body = body;

    return node;
}

AstNode *ast_init_noop(void) {
    AstNode *node = malloc(sizeof(struct AstNode));
    node->type = AST_NOOP;
    return node;
}

// parser functions
Parser parser_init(Lexer *lexer) {
    Parser parser;

    parser.lexer = lexer;
    parser.current_token = lexer_get_next_token(parser.lexer);

    return parser;
}

void parser_eat(Parser *self, int token_type) {
    if (self->current_token.type == token_type) {
        self->current_token = lexer_get_next_token(self->lexer);
    } else {
        printf("unexpected ");
        print_tokens(&(self->current_token));
        // exit(1);
    }
}

AstNode **parser_parse_prog(Parser *self, int *child_count) {
    AstNode **root = malloc(sizeof(struct AstNode *));
    *child_count = 0;

    AstNode *child = parser_parse_form(self);
    root[*child_count] = child;
    (*child_count)++;

    while (self->current_token.type != TOKEN_EOF) {
        child = parser_parse_form(self);     
        root = realloc(root, (*child_count + 1) * sizeof(struct AstNode *));
        root[*child_count] = child;
        (*child_count)++;
    }

    return root;
}

AstNode *parser_parse_form(Parser *self) {
    AstNode *node;

    if (self->current_token.type == TOKEN_LET) {
        node = parser_parse_assignment(self);
        parser_eat(self, TOKEN_SEMI);
    } else if (self->current_token.type == TOKEN_EOF ||
               self->current_token.type == TOKEN_SEMI) {
        if (self->current_token.type == TOKEN_EOF) {
            parser_eat(self, TOKEN_EOF);
        } else {
            parser_eat(self, TOKEN_SEMI);
            parser_eat(self, TOKEN_EOF);
        }
        node = ast_init_noop();
    } else {
        node = parser_parse_expr(self);
        parser_eat(self, TOKEN_SEMI);
    }

    return node;
}

AstNode *parser_parse_assignment(Parser *self) {
    AstNode *node;
    parser_eat(self, TOKEN_LET);

    AstNode *left = ast_init_var(self->current_token.value, self->current_token);
    parser_eat(self, TOKEN_IDENT);

    Token op = self->current_token;
    parser_eat(self, TOKEN_ASSIGN);

    node = ast_init_assign(left, parser_parse_expr(self), op);
    return node;
}

AstNode *parser_parse_expr(Parser *self) {
    AstNode *node;
    switch (self->current_token.type) {
        case TOKEN_IF:
            parser_eat(self, TOKEN_IF);

            AstNode *cond = parser_parse_logical_or(self);
            parser_eat(self, TOKEN_THEN);
            AstNode *then = parser_parse_expr(self);

            AstNode *alter = ast_init_nil();
            if (self->current_token.type == TOKEN_ELSE) {
                parser_eat(self, TOKEN_ELSE);
                alter = parser_parse_expr(self);
            }

            node = ast_init_if(cond, then, alter);
            break;
        case TOKEN_FN:
            parser_eat(self, TOKEN_FN); 
            parser_eat(self, TOKEN_LPAREN);

            int param_count = 0;
            struct AstNode **params = malloc(sizeof(struct Token *));

            while (self->current_token.type != TOKEN_RPAREN) {
                params[param_count] = parser_parse_primary(self); 
                param_count++;

                if (self->current_token.type == TOKEN_COMMA) {
                    parser_eat(self, TOKEN_COMMA);
                }
            }

            parser_eat(self, TOKEN_RPAREN);
            parser_eat(self, TOKEN_ARROW);

            node = ast_init_fn(
                params, param_count, parser_parse_expr(self));
            break;
        case TOKEN_PUTS:
            parser_eat(self, TOKEN_PUTS);
            parser_eat(self, TOKEN_LPAREN);
            node = ast_init_puts(parser_parse_expr(self));
            parser_eat(self, TOKEN_RPAREN);
            break;
        case TOKEN_DO:
            parser_eat(self, TOKEN_DO);
            int child_count = 0;
            struct AstNode **children = malloc(sizeof(struct AstNode *));

            while (self->current_token.type != TOKEN_DONE) {
                children[child_count] = parser_parse_form(self);
                child_count++;
            }
            
            parser_eat(self, TOKEN_DONE);
            node = ast_init_block(children, child_count);
            break;
        default:
            node = parser_parse_logical_or(self);
    }

    return node;
}

AstNode *parser_parse_logical_or(Parser *self) {
    AstNode *node = parser_parse_logical_and(self);
    while (self->current_token.type == TOKEN_OR) {
        Token token = self->current_token;
        parser_eat(self, TOKEN_OR);

        node = ast_init_binop(node, parser_parse_logical_and(self), token);
    }

    return node;
}

AstNode *parser_parse_logical_and(Parser *self) {
    AstNode *node = parser_parse_equality(self);
    while (self->current_token.type == TOKEN_AND) {
        Token token = self->current_token;
        parser_eat(self, TOKEN_AND);

        node = ast_init_binop(node, parser_parse_equality(self), token);
    }

    return node;
}

AstNode *parser_parse_equality(Parser *self) {
    AstNode *node = parser_parse_comparison(self);
    while (self->current_token.type == TOKEN_EQUAL ||
           self->current_token.type == TOKEN_NEQUAL) {
        Token token = self->current_token;
        
        if (token.type == TOKEN_EQUAL) {
            parser_eat(self, TOKEN_EQUAL);
        } else {
            parser_eat(self, TOKEN_NEQUAL);
        }

        node = ast_init_binop(node, parser_parse_comparison(self), token);
    }

    return node;
}

AstNode *parser_parse_comparison(Parser *self) {
    AstNode *node = parser_parse_addition(self);
    while (self->current_token.type == TOKEN_LT ||
           self->current_token.type == TOKEN_GT ||
           self->current_token.type == TOKEN_LTE ||
           self->current_token.type == TOKEN_GTE) {
        Token token = self->current_token;
        
        switch (token.type) {
            case TOKEN_LT: parser_eat(self, TOKEN_LT); break;
            case TOKEN_GT: parser_eat(self, TOKEN_GT); break;
            case TOKEN_LTE: parser_eat(self, TOKEN_LTE); break;
            case TOKEN_GTE: parser_eat(self, TOKEN_GTE); break;
        }

        node = ast_init_binop(node, parser_parse_addition(self), token);
    }

    return node;
}

AstNode *parser_parse_addition(Parser *self) {
    AstNode *node = parser_parse_multiplication(self);
    
    while (self->current_token.type == TOKEN_PLUS ||
           self->current_token.type == TOKEN_MINUS) {
        Token token = self->current_token;
        
        if (token.type == TOKEN_PLUS) {
            parser_eat(self, TOKEN_PLUS);
        } else {
            parser_eat(self, TOKEN_MINUS);
        }

        node = ast_init_binop(node, parser_parse_multiplication(self), token);
    }

    return node;
}

AstNode *parser_parse_multiplication(Parser *self) {
    AstNode *node = parser_parse_unary(self);
    
    while (self->current_token.type == TOKEN_MUL ||
           self->current_token.type == TOKEN_DIV ||
           self->current_token.type == TOKEN_MOD) {
        Token token = self->current_token;
        
        if (token.type == TOKEN_MUL) {
            parser_eat(self, TOKEN_MUL);
        } else if (token.type == TOKEN_DIV){
            parser_eat(self, TOKEN_DIV);
        } else {
            parser_eat(self, TOKEN_MOD);
        }

        node = ast_init_binop(node, parser_parse_unary(self), token);
    }

    return node;
}

AstNode *parser_parse_unary(Parser *self) {
    AstNode *node;
    
    Token token = self->current_token;
    switch (token.type) {
        case TOKEN_BANG:
            parser_eat(self, TOKEN_BANG);
            node = ast_init_unop(parser_parse_unary(self), token);
            break;
        case TOKEN_MINUS:
            parser_eat(self, TOKEN_MINUS);
            node = ast_init_unop(parser_parse_unary(self), token);
            break;
        default:
            node = parser_parse_call(self);
    }

    return node;
}

AstNode *parser_parse_call(Parser *self) {
    AstNode *node;
    node = parser_parse_primary(self);

    if (self->current_token.type == TOKEN_LPAREN) {
        parser_eat(self, TOKEN_LPAREN);
        // AstNode *fncall_node = ast_init_fncall(node->value.ident_name);

        int arg_count = 0;
        AstNode **args = malloc(sizeof(struct AstNode *));

        while (self->current_token.type != TOKEN_RPAREN) {
            args[arg_count] = parser_parse_expr(self);
            arg_count++;

            if (self->current_token.type == TOKEN_COMMA) {
                parser_eat(self, TOKEN_COMMA);
            }
        }

        parser_eat(self, TOKEN_RPAREN);
        node = ast_init_fncall(node->value.ident_name, args, arg_count, node);
    }
    
    return node;
}

AstNode *parser_parse_primary(Parser *self) {
    AstNode *node;
    Token token = self->current_token;
    double number;

    switch (token.type) {
        case TOKEN_NUMBER:
            parser_eat(self, TOKEN_NUMBER);
            sscanf(token.value, "%lf", &number);
            node = ast_init_num(number);
            break;
        case TOKEN_STRING:
            parser_eat(self, TOKEN_STRING);
            node = ast_init_str(token.value);
            break;
        case TOKEN_IDENT:
            parser_eat(self, TOKEN_IDENT);
            node = ast_init_var(token.value, token);
            break;
        case TOKEN_LPAREN:
            parser_eat(self, TOKEN_LPAREN);
            node = parser_parse_expr(self);
            parser_eat(self, TOKEN_RPAREN);
            break;
        case TOKEN_TRUE:
            parser_eat(self, TOKEN_TRUE);
            node = ast_init_bool(1);
            break;
        case TOKEN_FALSE:
            parser_eat(self, TOKEN_FALSE);
            node = ast_init_bool(0);
            break;
        case TOKEN_NIL:
            parser_eat(self, TOKEN_NIL);
            node = ast_init_nil();
            break;
        case TOKEN_GETS:
            parser_eat(self, TOKEN_GETS);
            parser_eat(self, TOKEN_LPAREN);
            parser_eat(self, TOKEN_RPAREN);
            char *get_str = builtin_gets();
            node = ast_init_str(get_str);
            break;
        case TOKEN_EOF:
            parser_eat(self, TOKEN_EOF);
            node = ast_init_noop();
            break;
    }

    return node;
}

// env functions
Env *create_env(Env *parent) {
    Env *env = malloc(sizeof(struct Env));

    env->records = NULL;
    env->parent = parent;
    env->records = NULL;

    return env;
}

void env_insert(Env **env, char *varname, AstNode *value) {
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

// builtin functions
void builtin_puts(AstNode *node) {
    switch (node->type) {
        case AST_NUMBER:
            if (fmod(node->value.num_value, 1) == 0) {
                printf("%d\n", (int)node->value.num_value);
            } else {
                printf("%lf\n", node->value.num_value);
            }
            break;
        case AST_STRING:
            printf("%s\n", node->value.str_value);
            break;
        case AST_BOOL:
            if (node->value.bool_value == 1) {
                printf("true\n");
            } else {
                printf("false\n");
            }
            break;
        case AST_NIL:
            puts("nil");
            break;
        case AST_FN:
            if (node->value.ident_name == NULL) {
                puts("<lambda expression>");
            } else {
                printf("<function %s>\n", node->value.ident_name);
            }
            break;
        default:
            printf("");
    }
}

char *builtin_gets(void) {
    char *result = NULL;
    size_t size = 0;
    ssize_t len = getline(&result, &size, stdin);

    if (len == -1) {
        puts("error reading input");
        exit(1);
    }

    result[len - 1] = '\0';
    return result;
}

// visitor functions
AstNode *visitor_visit_root(struct AstNode **root, int child_count, Env *env) {
    AstNode *node;

    for (int i = 0; i < child_count; i++) {
        node = visitor_visit_node(root[i], env);
    }

    return node;
}

AstNode *visitor_visit_node(AstNode *node, Env *env) {
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
        case AST_PUTS:
            builtin_puts(visitor_visit_node(node->body, env));
            return ast_init_noop();
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

int visitor_seek_truth(AstNode *node) {
    if (node->type == AST_NIL) return 0;
    else if (node->type == AST_BOOL) return node->value.bool_value;
    return 1;
}

AstNode *visitor_visit_assignment(AstNode *node, Env *env) {
    env_insert(&env, node->left->value.ident_name, node->right);
    return ast_init_noop();
}

AstNode *visitor_visit_var(AstNode *node, Env *env) {
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

AstNode *visitor_visit_if(AstNode *node, Env *env) {
    AstNode *cond = visitor_visit_node(node->condition, env);
    int truth = visitor_seek_truth(cond);

    if (truth) {
        return visitor_visit_node(node->then_branch, env);
    }

    return visitor_visit_node(node->else_branch, env);
}

AstNode *visitor_visit_binop(AstNode *node, Env *env) {
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

AstNode *visitor_visit_unop(AstNode *node, Env *env) {
    AstNode *result = visitor_visit_node(node->right, env);

    if (node->op.type == TOKEN_BANG) {
        result->value.bool_value = result->value.bool_value == 1 ? 0 : 1;
    } else if (node->op.type == TOKEN_MINUS) {
        result->value.num_value *= -1;
    }

    return result; 
}

AstNode *visitor_visit_block(AstNode *node, Env *env) {
    AstNode *expr;
    Env *local_env = create_env(env);
    
    for (int i = 0; i < node->child_count; i++) {
        expr = visitor_visit_node(node->children[i], local_env);
    }
    
    return expr;
}

AstNode *visitor_visit_fncall(AstNode *node, Env *env) {
    // check if function exists
    if (node->value.ident_name != NULL) {
        if (env_check_var(env, node->value.ident_name) == 0) {
            printf("func \"%s\" is not defined on line %d\n",
                   node->value.ident_name, node->token.line);
            exit(1);
        } 
        // get fn ast node
        AstNode *fn = env_find_var(env, node->value.ident_name);

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
            env_insert(&local_env, fn->params[i]->value.ident_name, arg);
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
        env_insert(
            &local_env, node->lambda->params[i]->value.ident_name, arg);
    }

    return visitor_visit_node(node->lambda->body, local_env);
}

// debugging
void debug_print_tokens(Lexer *lexer) {
    Token token;
    do {
        token = lexer_get_next_token(lexer);
        print_tokens(&token);
    } while (token.type != TOKEN_EOF);
}

void debug_print_ast(AstNode **root, int child_count) {
    printf("stmt count: %d\n", child_count);
    for (int i = 0; i < child_count; i++) {
        print_ast(root[i]);
    }
}

