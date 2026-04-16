#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include "tokens.h"

#define MAGENTA     "\033[35m"
#define RED         "\033[31m"
#define BLUE        "\033[34m"
#define RESET       "\033[0m"
#define FETT        "\033[1m"

typedef struct {
    const char *input;
    int line;
    int column;
    int pos;
} Lexer;

// Keywords Map
QTokenType check_keyword(const char *str) {
    if (strcmp(str, "int") == 0) return TOKEN_INT;
    if (strcmp(str, "float") == 0) return TOKEN_FLOAT;
    if (strcmp(str, "double") == 0) return TOKEN_DOUBLE;
    if (strcmp(str, "short") == 0) return TOKEN_SHORT;
    if (strcmp(str, "byte") == 0) return TOKEN_BYTE;
    if (strcmp(str, "func") == 0) return TOKEN_FUNC;
    if (strcmp(str, "ret") == 0) return TOKEN_RET;
    if (strcmp(str, "asm") == 0) return TOKEN_ASM;
    if (strcmp(str, "true") == 0) return TOKEN_TRUE;
    if (strcmp(str, "false") == 0) return TOKEN_FALSE;
    return TOKEN_ID;
}

Token make_token(QTokenType type, const char *name, double value, int line, int column) {
    Token t;
    t.type = type;
    t.value = value;
    t.line = line;
    t.column = column;
    strncpy(t.name, name, 127);
    t.name[127] = '\0';
    return t;
}

Token get_next_token(Lexer *lexer) {
    const char *input = lexer->input + lexer->pos;
    int line = lexer->line;
    int column = lexer->column;

    // Skip whitespace (aber nicht newline!)
    while (*input && isspace(*input) && *input != '\n') {
        if (*input == '\t') {
            lexer->column += 4;
        } else {
            lexer->column++;
        }
        input++;
    }

    lexer->input = input - lexer->pos;
    lexer->pos = 0;

    // Numbers
    if (isdigit(*input)) {
        double value = 0;
        int start = lexer->column;
        
        while (isdigit(*input)) {
            value = value * 10 + (*input - '0');
            input++;
            lexer->column++;
        }

        // Decimals
        if (*input == '.') {
            input++;
            lexer->column++;
            double factor = 0.1;
            while (isdigit(*input)) {
                value += (*input - '0') * factor;
                factor *= 0.1;
                input++;
                lexer->column++;
            }
        }

        char buffer[64];
        snprintf(buffer, 64, "%.10g", value);
        
        lexer->pos = input - lexer->input;
        return make_token(TOKEN_NUMBER, buffer, value, line, start);
    }

    // Strings
    if (*input == '"' || *input == '\'') {
        char quote = *input;
        int start = lexer->column;
        input++;
        lexer->column++;

        char buffer[128] = {0};
        int i = 0;
        
        while (*input && *input != quote && i < 127) {
            buffer[i++] = *input;
            input++;
            lexer->column++;
        }

        if (*input == quote) {
            input++;
            lexer->column++;
        } else {
            fprintf(stderr, RED "Error at line %d: Unterminated string\n" RESET, line);
        }

        lexer->pos = input - lexer->input;
        return make_token(TOKEN_STR, buffer, 0, line, start);
    }

    // Comments
    if (*input == ';') {
        int start = lexer->column;
        input++;
        lexer->column++;

        char buffer[512] = {0};
        int i = 0;
        
        while (*input && *input != '\n' && i < 511) {
            buffer[i++] = *input;
            input++;
            lexer->column++;
        }

        lexer->pos = input - lexer->input;
        return make_token(TOKEN_COMMENT, buffer, 0, line, start);
    }

    // Identifiers & Keywords
    if (isalpha(*input) || *input == '_') {
        int start = lexer->column;
        char buffer[128] = {0};
        int i = 0;

        while ((isalnum(*input) || *input == '_') && i < 127) {
            buffer[i++] = *input;
            input++;
            lexer->column++;
        }
        buffer[i] = '\0';

        QTokenType type = check_keyword(buffer);
        lexer->pos = input - lexer->input;
        return make_token(type, buffer, 0, line, start);
    }

    // Operators (2-char first)
    if (*input == '-' && *(input+1) == '>') {
        lexer->pos = (input + 2) - lexer->input;
        lexer->column += 2;
        return make_token(TOKEN_ARROWR, "->", 0, line, column);
    } else if (*input == '<' && *(input+1) == '-') {
        lexer->pos = (input + 2) - lexer->input;
        lexer->column += 2;
        return make_token(TOKEN_ARROWL, "<-", 0, line, column);
    }

    if (*input == '+' && *(input+1) == '+') {
        lexer->pos = (input + 2) - lexer->input;
        lexer->column += 2;
        return make_token(TOKEN_INC, "++", 0, line, column);
    }

    if (*input == '-' && *(input+1) == '-') {
        lexer->pos = (input + 2) - lexer->input;
        lexer->column += 2;
        return make_token(TOKEN_DEC, "--", 0, line, column);
    }

    // Single-char operators
    int start_col = lexer->column;
    char c = *input;
    input++;
    lexer->column++;
    lexer->pos = input - lexer->input;

    switch (c) {
        case '+': return make_token(TOKEN_PLUS, "+", 0, line, start_col);
        case '-': return make_token(TOKEN_MINUS, "-", 0, line, start_col);
        case '*': return make_token(TOKEN_MUL, "*", 0, line, start_col);
        case '/': return make_token(TOKEN_DIV, "/", 0, line, start_col);
        case '&': return make_token(TOKEN_AND, "&", 0, line, start_col);
        case ':': return make_token(TOKEN_OR, ":", 0, line, start_col);
        case '^': return make_token(TOKEN_XOR, "^", 0, line, start_col);
        case '!': return make_token(TOKEN_NOT, "!", 0, line, start_col);
        case '>': return make_token(TOKEN_GT, ">", 0, line, start_col);
        case '<': return make_token(TOKEN_LT, "<", 0, line, start_col);
        case '=': return make_token(TOKEN_EQUAL, "=", 0, line, start_col);
        case '@': return make_token(TOKEN_CALL, "@", 0, line, start_col);
        case '(': return make_token(TOKEN_LPAREN, "(", 0, line, start_col);
        case ')': return make_token(TOKEN_RPAREN, ")", 0, line, start_col);
        case '{': return make_token(TOKEN_LBRACE, "{", 0, line, start_col);
        case '}': return make_token(TOKEN_RBRACE, "}", 0, line, start_col);
        case '[': return make_token(TOKEN_LBRACK, "[", 0, line, start_col);
        case ']': return make_token(TOKEN_RBRACK, "]", 0, line, start_col);
        case '#': return make_token(TOKEN_SEC, "#", 0, line, start_col);
        case ',': return make_token(TOKEN_COMMA, ",", 0, line, start_col);
        case '\n': 
            lexer->line++;
            lexer->column = 0;
            return make_token(TOKEN_NEWLINE, "\\n", 0, line, start_col);
        case '\0': return make_token(TOKEN_EOF, "EOF", 0, line, start_col);
        default: 
            return make_token(TOKEN_UNKNOWN, "", 0, line, start_col);
    }
}

Token* tokenize(const char *input, int *token_count) {
    Token *tokens = malloc(sizeof(Token) * 10);
    int capacity = 10;
    *token_count = 0;

    Lexer lexer = {
        .input = input,
        .line = 1,
        .column = 0,
        .pos = 0
    };

    Token token;
    do {
        token = get_next_token(&lexer);
        
        // Skip comments und newlines (optional)
        if (token.type == TOKEN_COMMENT || token.type == TOKEN_NEWLINE) {
            continue;
        }

        if (*token_count >= capacity) {
            capacity *= 2;
            tokens = realloc(tokens, sizeof(Token) * capacity);
        }

        tokens[*token_count] = token;
        (*token_count)++;

    } while (token.type != TOKEN_EOF);

    return tokens;
}

void printTok(Token *tokens, int count) {
    printf(FETT MAGENTA "=== TOKENS ===" RESET "\n");
    for (int i = 0; i < count; i++) {
        printf("[%d:%d] %s: ", tokens[i].line, tokens[i].column, tokens[i].name);
        
        switch (tokens[i].type) {
            case TOKEN_NUMBER: printf("NUMBER (%.2f)\n", tokens[i].value); break;
            case TOKEN_STR: printf("STRING\n"); break;
            case TOKEN_ARROWR: printf("ARROW\n"); break;
            case TOKEN_ARROWL: printf("ARROW\n"); break;
            case TOKEN_PLUS: printf("PLUS\n"); break;
            case TOKEN_INT: printf("KEYWORD INT\n"); break;
            case TOKEN_FLOAT: printf("KEYWORD FLOAT\n"); break;
            case TOKEN_ID: printf("IDENTIFIER\n"); break;
            default: printf("TOKEN TYPE %d\n", tokens[i].type);
        }
    }
}

// Test
int main(int argc, char *argv[]) {
    const char *code = 
        "10 -> int x\n"
        "5.5 -> float b\n"
        "1 + 1 -> float result\n"
        "x <- b\n"
        "x++\n"
        "?(x > 5) { x-- }\n"
        "ret 0\n";

    int count = 0;
    Token *tokens = tokenize(code, &count);

    free(tokens);
    return 0;
}