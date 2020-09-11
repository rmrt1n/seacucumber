#ifndef LEXER_H
#define LEXER_H

// token structure, contains token type, lexeme and its line number
typedef struct Token {
    enum {
        // literals and identifier
        TOKEN_NUMBER, TOKEN_STRING, TOKEN_IDENT,

        // keywords
        TOKEN_LET, TOKEN_AND, TOKEN_OR,
        TOKEN_IF, TOKEN_THEN, TOKEN_ELSE,
        TOKEN_DO, TOKEN_DONE, TOKEN_FN,
        TOKEN_TRUE, TOKEN_FALSE, TOKEN_NIL,

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

// lexer structure, contains source code, position of current character,
// and the character itself
typedef struct Lexer {
    char *contents;
    int pos;
    char current_char;
    int line;
} Lexer;

// initialize lexer with source code
Lexer lexer_init(char *contents);
// get next token function
Token lexer_get_next_token(Lexer *self);

#endif
