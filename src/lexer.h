#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Farben (optional)
#define MAGENTA "\033[35m"
#define RED     "\033[31m"
#define BLUE    "\033[34m"
#define RESET   "\033[0m"
#define FETT    "\033[1m"

// =====================
// TOKEN TYPES
// =====================
typedef enum {
    // Literals
    TOKEN_NUMBER,
    TOKEN_STR,
    TOKEN_TRUE,
    TOKEN_FALSE,
    
    // Keywords
    TOKEN_FUNC,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_DOUBLE,
    TOKEN_SHORT,
    TOKEN_BYTE,
    TOKEN_DEF,
    TOKEN_RET,
    TOKEN_VAR,
    TOKEN_ASM,

    // Operators
    TOKEN_CALL,     // @
    TOKEN_SEC,      // #
    TOKEN_ARROWR,   // ->
    TOKEN_ARROWL,   // <-
    TOKEN_PLUS,     // +
    TOKEN_MINUS,    // -
    TOKEN_MUL,      // *
    TOKEN_DIV,      // /
    TOKEN_AND,      // &
    TOKEN_OR,       // :
    TOKEN_XOR,      // ^
    TOKEN_NOT,      // !
    TOKEN_GT,       // >
    TOKEN_LT,       // <
    TOKEN_EQUAL,    // =
    TOKEN_INC,      // ++
    TOKEN_DEC,      // --

    // Delimiters
    TOKEN_LPAREN,   // (
    TOKEN_RPAREN,   // )
    TOKEN_LBRACE,   // {
    TOKEN_RBRACE,   // }
    TOKEN_LBRACK,   // [
    TOKEN_RBRACK,   // ]
    TOKEN_SEMICOLON,// ;
    TOKEN_COMMA,    // ,
    TOKEN_COLON,    // :

    // Special
    TOKEN_ID,       // identifier
    TOKEN_COMMENT,
    TOKEN_TAB,
    TOKEN_NEWLINE,
    TOKEN_EOF,
    TOKEN_UNKNOWN
} QTokenType;

// =====================
// STRUCTS
// =====================
typedef struct {
    QTokenType type;
    double value;
    char name[128];
    int line;
    int column;
} Token;

typedef struct {
    const char *input;
    int line;
    int column;
    int pos;
} Lexer;

// =====================
// FUNCTION DECLARATIONS
// =====================
QTokenType check_keyword(const char *str);

Token make_token(QTokenType type, const char *name, double value, int line, int column);

Token get_next_token(Lexer *lexer);

Token* tokenize(const char *input, int *token_count);

void printTok(Token *tokens, int count);

#endif