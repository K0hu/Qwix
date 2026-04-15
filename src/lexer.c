
// Structure of Token types
typedef enum {
    TOKEN_NUMBER,
    TOKEN_EOF,
    TOKEN_UNKNOWN,
    TOKEN_EQUAL,
    TOKEN_GRT,
    TOKEN_LES,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_CLAMP,
    TOKEN_SUB,
    TOKEN_CMP,
    TOKEN_DEF,
    TOKEN_NOT,
    TOKEN_TAB,
    TOKEN_STR,
    TOKEN_POP,
    TOKEN_INT,
    TOKEN_SCMP,
    TOKEN_INC,
    TOKEN_JMP,
    TOKEN_VAR,
    TOKEN_MCLAM,
    TOKEN_AND,
    TOKEN_RET,
    TOKEN_COM,
    TOKEN_END,
    TOKEN_ASM,
    TOKEN_BSS,
    TOKEN_ECLAM,
    TOKEN_ARG,
    TOKEN_FLOAT,
    TOKEN_CALL,
    TOKEN_MOV
} QTokenType;

typedef struct {
    QTokenType type;
    int value;
    char name[128];
} Token;

// Tokenizer
Token get_next_token(const char **input) {
    if (strncmp(*input, "    ", 4) == 0) {
        *input += 4;
        return (Token){ TOKEN_TAB, 0 };
    }

    while (isspace(**input)) (*input)++;

    // sonst: Integer checken
    if (isdigit(**input)) {
        int value = 0;
        while (isdigit(**input)) {
            value = value * 10 + (**input - '0');
            (*input)++;
        }
        Token t = { TOKEN_NUMBER, value };
        snprintf(t.name, sizeof(t.name), "%d", value);
        t.dif = TOK_INT;
        return t;
    }

    if (**input == '\0') {
        return (Token){ TOKEN_EOF, 0 };
    }

    if (**input == '"' || **input == '\'') {
        char quote = **input;
        (*input)++;
        int i = 0;
        char buffer[128] = {0};
        while (**input != quote && **input != '\0' && i < 127) {
            buffer[i++] = **input;
            (*input)++;
        }
        errno = EINVAL;
        if (**input == quote) {(*input)++;} else {perror(RED "Error: Syntax error near string" RESET);} // schließendes Anführungszeichen überspringen
        Token t = { TOKEN_STR, 0 };
        strncpy(t.name, buffer, 127);
        t.dif = TOK_STR;
        return t;
    }

    if (**input == ';') {
        (*input)++;
        int i = 0;
        char buffer[128] = {0};
        while (**input != '\0' && i < 127) {
            buffer[i++] = **input;
            (*input)++;
        }
        Token t = { TOKEN_COM, 0 };
        strncpy(t.name, buffer, 127);

        return t;
    }
}