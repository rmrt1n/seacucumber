#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

// create token
static Token create_token(int token_type, char *value, int line) {
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

// advance to next character in source code
static void lexer_advance(Lexer *self) {
    if (self->pos < strlen(self->contents)) {
        if (self->current_char == '\n') self->line++;

        self->pos++;
        self->current_char = self->contents[self->pos];
    }
}

// advance position while current character is whitespace
static void lexer_skip_whitespace(Lexer *self) {
    while (isspace(self->current_char)) {
        lexer_advance(self);
    }
}

// ignore comments
static void lexer_ignore_comments(Lexer *self) {
    while (self->current_char != '\n') lexer_advance(self);
    lexer_skip_whitespace(self);
    while (self->current_char == '#') lexer_ignore_comments(self);
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
            } else if (strcmp(ident, "do") == 0) {
                token_type = TOKEN_DO;
            } else if (strcmp(ident, "done") == 0) {
                token_type = TOKEN_DONE;
            }

            return create_token(token_type, ident, self->line);
        }

        // get number token
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

        // for symbols and string
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

    // finally return eof token
    return create_token(TOKEN_EOF, "\0", self->line - 1);
}

