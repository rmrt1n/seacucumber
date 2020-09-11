#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "builtin.h"
#include "debug.h"

// functions here are recursive, so declared first
static AstNode *parser_parse_form(Parser *self);
static AstNode *parser_parse_assignment(Parser *self);
static AstNode *parser_parse_expr(Parser *self);
static AstNode *parser_parse_logical_or(Parser *self);
static AstNode *parser_parse_logical_and(Parser *self);
static AstNode *parser_parse_equality(Parser *self);
static AstNode *parser_parse_comparison(Parser *self);
static AstNode *parser_parse_addition(Parser *self);
static AstNode *parser_parse_multiplication(Parser *self);
static AstNode *parser_parse_unary(Parser *self);
static AstNode *parser_parse_call(Parser *self);
static AstNode *parser_parse_primary(Parser *self);

Parser parser_init(Lexer *lexer) {
    Parser parser;

    parser.lexer = lexer;
    parser.current_token = lexer_get_next_token(parser.lexer);

    return parser;
}

// expect token of type 'token_type', if not token then error
void parser_eat(Parser *self, int token_type) {
    if (self->current_token.type == token_type) {
        self->current_token = lexer_get_next_token(self->lexer);
    } else {
        printf("unexpected ");
        print_tokens(&(self->current_token));
        // exit(1);
    }
}

// parse every token until eof
// full grammar in ../bnf
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

// grammar for form -> (expression | assignment)
static AstNode *parser_parse_form(Parser *self) {
    AstNode *node;

    switch (self->current_token.type) {
        case TOKEN_LET:
            node = parser_parse_assignment(self);
            break;
        case TOKEN_EOF:
            parser_eat(self, TOKEN_EOF);
            node = ast_init_noop();
            break;
        default:
            node = parser_parse_expr(self);
    }

    return node;
}

// grammar for assignment -> 'let' ident = expression
static AstNode *parser_parse_assignment(Parser *self) {
    AstNode *node;
    parser_eat(self, TOKEN_LET);

    AstNode *left = ast_init_var(self->current_token.value, self->current_token);
    parser_eat(self, TOKEN_IDENT);

    Token op = self->current_token;
    parser_eat(self, TOKEN_ASSIGN);

    node = ast_init_assign(left, parser_parse_expr(self), op);
    return node;
}

// grammar for expression ->
// | 'if' logic_or 'then' expression ('else' expression)?  <- if expression
// | 'fn' '('params?')' '->' expression  <- function definition / lambda expr
// | block  <- block expression, evaluates to last expr in block
// | logic_or
static AstNode *parser_parse_expr(Parser *self) {
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
        case TOKEN_DO:
            parser_eat(self, TOKEN_DO);
            int child_count = 0;
            struct AstNode **children = malloc(sizeof(struct AstNode *));

            while (self->current_token.type != TOKEN_DONE) {
                children[child_count] = parser_parse_form(self);
                child_count++;
                parser_eat(self, TOKEN_SEMI);
            }
            
            parser_eat(self, TOKEN_DONE);
            node = ast_init_block(children, child_count);
            break;
        default:
            node = parser_parse_logical_or(self);
    }

    return node;
}

// grammar -> logic_and ('or' logic_and)*
static AstNode *parser_parse_logical_or(Parser *self) {
    AstNode *node = parser_parse_logical_and(self);
    while (self->current_token.type == TOKEN_OR) {
        Token token = self->current_token;
        parser_eat(self, TOKEN_OR);

        node = ast_init_binop(node, parser_parse_logical_and(self), token);
    }

    return node;
}

// grammar -> equality ('and' equality)*
static AstNode *parser_parse_logical_and(Parser *self) {
    AstNode *node = parser_parse_equality(self);
    while (self->current_token.type == TOKEN_AND) {
        Token token = self->current_token;
        parser_eat(self, TOKEN_AND);

        node = ast_init_binop(node, parser_parse_equality(self), token);
    }

    return node;
}

// grammar -> comparison (('==' | '!=') comparison)*
static AstNode *parser_parse_equality(Parser *self) {
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

// grammar -> addition (('<' | '>' | '<=' | '>=') addition)*
static AstNode *parser_parse_comparison(Parser *self) {
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

// grammar -> multiplication (('+' | '-') multiplication)*
static AstNode *parser_parse_addition(Parser *self) {
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

// grammar -> unary (('*' | '/') unary)*
static AstNode *parser_parse_multiplication(Parser *self) {
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

// grammar -> ('!' | '-') unary | call
static AstNode *parser_parse_unary(Parser *self) {
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

// grammar -> primary ('('args?')')*
static AstNode *parser_parse_call(Parser *self) {
    AstNode *node;
    node = parser_parse_primary(self);

    if (self->current_token.type == TOKEN_LPAREN) {
        parser_eat(self, TOKEN_LPAREN);

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

// number | string | ident | true | false | nil | '('expression')'
static AstNode *parser_parse_primary(Parser *self) {
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
        case TOKEN_EOF:
            parser_eat(self, TOKEN_EOF);
            node = ast_init_noop();
            break;
        default:
            printf("unexpected token ");
            print_tokens(&self->current_token);
            exit(1);
    }

    return node;
}

