#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <libgen.h>
#include "tinyexpr.h"
#include <math.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#define MAX_LINES 1000
#define MAX_LINE_LEN 1000

#define MAGENTA     "\033[35m"
#define RED     "\033[31m"
#define BLUE    "\033[34m"
#define RESET   "\033[0m"
#define FETT    "\033[1m"

// Adds a string to an string
char* append(char* str, const char* add_str) {
    int len = str ? strlen(str) : 0;
    int add_len = strlen(add_str);
    char* new_str = realloc(str, len + add_len + 1);
    if (!new_str) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    strcpy(new_str + len, add_str);  // Append the new string
    return new_str;
}

void replace(char *str, char find, char replace) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == find) {
            str[i] = replace;
        }
    }
}


bool is_integer(double value) {
    const double eps = 1e-9;
    return fabs(value - round(value)) < eps;
}

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

typedef enum {
    REG_EAX,
    REG_EBX,
    REG_ECX,
    REG_EDX,
    REG_ESI,
    REG_EDI,
    REG_ESP,
    REG_EBP,
    EBP_COUNT
} RegisterName;

// Type of variables
typedef enum {
    TOK_DOUBLE,
    TOK_FLOAT,
    TOK_INT,
    TOK_STR,
    TOK_UND
} Type;

// Token structures
typedef struct {
    QTokenType type;
    int value;
    char name[128];
    Type dif;
} Token;

// Variables structure
typedef struct {
    char name[32];
    int value;
    char* text;
    Type type;
} Variable;

typedef struct {
    char fileName[32];
} Incl;

// Proofs if value is in array
bool is_in_array(Variable* array, int count, const char* value) {
    for (int i = 0; i < count; i++) {
        if (strcmp(array[i].name, value) == 0) {
            return true;  // Wert gefunden
        }
    }
    return false;  // Wert nicht gefunden
}

// Gets text of var
char* getChar(const char* name, Variable* vars, int count) {
    for (int i = 0; i < count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            return vars[i].text;
        }
    }
}

// Gets value of var
int getValue(const char* name, Variable* vars, int count) {
    for (int i = 0; i < count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            return vars[i].value;
        }
    }
}

// Updates variable
void updateVar(Variable* vars, int count, const char* name, int value, char* text) {
    for (int i = 0; i < count; i++) {
        if (strcmp(vars[i].name, name) == 0) {
            vars[i].value = value;
            vars[i].text = text;
            return;
        }
    }
}

// Adds var to list
Variable* addVar(Variable* vars, int *count, const char* name, int value, char* text, Type type) {
    vars = realloc(vars, sizeof(Variable) * (*count + 1));
    strcpy(vars[*count].name, name);
    vars[*count].value = value;
    vars[*count].text = text;
    vars[*count].type = type;
    (*count)++;
    return vars;
}

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


    if (**input == '+') {
        (*input)++;
        if (**input == '+') {
            (*input)++;
            return (Token){ TOKEN_INC, 0 };
        } else {
            return (Token){ TOKEN_POP, 0 };
        }
    }

    if (**input == '=') { (*input)++; return (Token){ TOKEN_MOV, 0 }; }
    if (**input == '@') { (*input)++; return (Token){ TOKEN_CALL, 0 }; }
    if (**input == '-') { (*input)++; return (Token){ TOKEN_SUB, 0 }; }
    if (**input == ':') { (*input)++; return (Token){ TOKEN_JMP, 0 }; }
    if (**input == '|') { (*input)++; return (Token){ TOKEN_BSS, 0 }; }
    if (**input == '.') { (*input)++; return (Token){ TOKEN_DEF, 0 }; }
    if (**input == '/') { (*input)++; return (Token){ TOKEN_RET, 0 }; }
    if (**input == '\t') { (*input)++; return (Token){ TOKEN_TAB, 0 }; }
    if (**input == '%') { (*input)++; return (Token){ TOKEN_CMP, 0 }; }
    if (**input == '&') { (*input)++; return (Token){ TOKEN_SCMP, 0 }; }
    if (**input == '?') { (*input)++; return (Token){ TOKEN_EQUAL, 0 }; }
    if (**input == '!') { (*input)++; return (Token){ TOKEN_NOT, 0 }; }
    if (**input == '>') { (*input)++; return (Token){ TOKEN_GRT, 0 }; }
    if (**input == '<') { (*input)++; return (Token){ TOKEN_LES, 0 }; }
    if (**input == '*') { (*input)++; return (Token){ TOKEN_END, 0 }; }
    if (**input == '#') { (*input)++; return (Token){ TOKEN_VAR, 0 }; }
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

    if (**input == '$') {
        (*input)++;
        int value = 0;
        if (isdigit(**input)) {
            value = (value * 10 + (**input - '0')) * 4;
            (*input)++;
        }
        Token t = { TOKEN_ARG, value };
        snprintf(t.name, sizeof(t.name), "[esp+%d]", value);
        return t;
    }

    if (**input == '~') {
        char quote = '~';
        (*input)++;
        int i = 0;
        char buffer[128] = {0};
        while (**input != quote && **input != '\0' && i < 127) {
            buffer[i++] = **input;
            (*input)++;
        }
        if (**input == quote) (*input)++;
        Token t = { TOKEN_ASM, 0 };
        strncpy(t.name, buffer, 127);

        return t;
    }

    if (**input == '{') {
        char quote = '}';
        (*input)++;
        int i = 0;
        char buffer[128] = {0};
        while (**input != quote && **input != '\0' && i < 127) {
            buffer[i++] = **input;
            (*input)++;
        }
        if (**input == quote) (*input)++;
        Token t = { TOKEN_INT, 0 };
        strncpy(t.name, buffer, 127);
        t.dif = TOK_INT;
        return t;
    }


    if (**input == '(') {
        char quote = ')';
        int i = 0;
        char buffer[128] = {0};
        buffer[i++] = '[';
        (*input)++;
        while (**input != quote && **input != '\0' && i < 127) {
            buffer[i++] = **input;
            (*input)++;
        }
        buffer[i++] = ']';
        (*input)++;
        Token t = { TOKEN_CLAMP, 0 };
        strncpy(t.name, buffer, 127);
        return t;
    }

    if (**input == '[') {
    (*input)++;  // skip '['

    const char* start = *input;
    const char* p = *input;
    const char* lastBracket = NULL;

    while (*p != '\0') {
        if (*p == ']')
            lastBracket = p;   // merke JEDE Position eines ']' 
        p++;
    }

    // Kein ']' gefunden?
    if (!lastBracket) {
            // Dann ist alles vom '[' bis EOF der Inhalt
            Token t = { TOKEN_ECLAM, 0 };
            strncpy(t.name, start, sizeof(t.name)-1);
            *input = p; // bis zum Ende
            return t;
        }

        // Länge bis zum letzten ']'
        int length = lastBracket - start;

        Token t = { TOKEN_ECLAM, 0 };
        if (length >= sizeof(t.name))
            length = sizeof(t.name)-1;

        strncpy(t.name, start, length);
        t.name[length] = '\0';

        *input = lastBracket + 1;  // hinter das letzte ]

        return t;
    }

    char name[32] = { 0 };
    int i = 0;
    while (isalpha(**input) || isdigit(**input) || **input == '_') {
        if (i < (int)(sizeof(name) - 1)) {
            name[i++] = **input;
        }
        (*input)++;
    }

    if (i > 0) {
        name[i] = '\0';
        Token token = { TOKEN_UNKNOWN, 0 };
        strncpy(token.name, name, sizeof(token.name) - 1);
        return token;
    }

    (*input)++;
    return (Token){ TOKEN_UNKNOWN, 0 };
}

// Token to string
const char* pToken(QTokenType type) {
    switch (type) {
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_EOF: return "EOF";
        case TOKEN_UNKNOWN: return "UNKNOWN";
        case TOKEN_EQUAL: return "EQUAL";
        case TOKEN_GRT: return "GRT";
        case TOKEN_LES: return "LES";
        case TOKEN_TRUE: return "TRUE";
        case TOKEN_FALSE: return "FALSE";
        case TOKEN_CLAMP: return "CLAMP";
        case TOKEN_CMP: return "CMP";
        case TOKEN_DEF: return "DEF";
        case TOKEN_NOT: return "NOT";
        case TOKEN_TAB: return "TAB";
        case TOKEN_COM: return "COMMENT";
        case TOKEN_STR: return "STR";
        case TOKEN_SCMP: return "SCMP";
        case TOKEN_INC: return "INC";
        case TOKEN_INT:   return "TOK_INT";
        default: return "INVALID";
    }
}

// Gets all tokens
Token* inter(const char *input, int *token_count, bool tok, bool *randomint, bool *bss) {
    Token *tokens = malloc(sizeof(Token) * 10);
    int token_capacity = 10;

    Token token;
    *token_count = 0;

    do {
        token = get_next_token(&input);
        if (token.type == TOKEN_BSS) {*bss = true;}
        if (!strcmp(basename(token.name), "randint") && !strcmp(tokens[*token_count - 1].name,"include")) {*randomint = true;}
        if (tok) {
            printf("Token: %-10s", pToken(token.type));
            if (token.type == TOKEN_NUMBER)
                printf("  Value: %d\n", token.value);
            else if (token.name[0] != '\0')
                printf("  Name: %s\n", token.name);
            else
                printf("\n");
        }
        if (*token_count >= token_capacity) {
            token_capacity *= 2;
            tokens = realloc(tokens, sizeof(Token) * token_capacity);
        }

        tokens[*token_count] = token;
        (*token_count)++;

    } while (token.type != TOKEN_EOF);

    return tokens;
}

// Replaces vars with its value in math formels
void replace_all_vars(char *expr, Variable *vars, int var_count) {
    char buffer[512]; // temporärer Puffer, groß genug dimensionieren
    int replaced;

    do {
        replaced = 0;
        for (int k = 0; k < var_count; k++) {
            char *pos = strstr(expr, vars[k].name);
            if (pos != NULL) {
                buffer[0] = '\0';
                strncat(buffer, expr, pos - expr);
                char value[64]; 
                sprintf(value, "%d", (int)vars[k].value);
                strcat(buffer, value);
                strcat(buffer, pos + strlen(vars[k].name));
                strcpy(expr, buffer);

                replaced = 1;
                break;
            }
        }
    } while (replaced);
}


// Logic for the tokens, creates the NASM-code
char* parser(Token* tokens, int *token_count, char **incl, bool nw, bool ri, bool bssb) {
    char* code = malloc(1); 
    code[0] = '\0'; 
    char* data = malloc(1); 
    data[0] = '\0'; 
    char* text = malloc(1); 
    text[0] = '\0'; 
    char* jmp = malloc(1); 
    jmp[0] = '\0'; 
    char* bss = malloc(1); 
    bss[0] = '\0';
    char* df = malloc(1); 
    df[0] = '\0';

    char formatted[512];
    char error[512];
    char loop[32];
    char ic[64];

    Variable* variables = NULL;
    int var_count = 0;
    variables = addVar(variables, &var_count, "eax", 0, "", TOK_UND);
    int EOF_counter = 1;

    Variable* jumps = NULL;
    int jmp_count = 0;

    char* expr;
    double value;

    int current_section = 0;
    srand(time(NULL));

    if (bssb) {
        bss = append(bss, "section .bss\n    ");
    }
    df = append(df, "\n");
#ifdef _WIN32
        text = append(text, "section .text\n    global _start\n    extern _printf, strcmp, srand, GetTickCount, gets, _atoi");
#else
        text = append(text, "section .text\n    global main\n    extern printf, atoi, fgets, time");
#endif

    data = append(data, "section .data\n    ");
    if (ri) {
#ifdef _WIN32
            code = append(code, "_start:\n    push 0\n    call GetTickCount\n    add esp, 4\n    push eax\n    call srand\n    add esp, 4\n    ");
#else
            code = append(code, "main:\n    push 0\n    call time\n    add esp, 4\n    push eax\n    call srand\n    add esp, 4\n    ");
#endif
        
    } else {
#ifdef _WIN32
        code = append(code, "_start:\n    ");
#else
        code = append(code, "main:\n    ");
#endif
    }

    for (int i = 0; i < *token_count; i++) {
        Token t = tokens[i];

        switch (t.type)
        {
        case TOKEN_TAB:
            current_section = 1;
            break;
        
        case TOKEN_EOF:
            current_section = 0;
            break;
        
        default:
            break;
        }

        switch (t.type) {
            case TOKEN_CALL:
                snprintf(formatted, sizeof(formatted), "call %s\n    ", tokens[i + 1].name);
                if (!is_in_array(jumps, jmp_count, tokens[i + 2].name)) {
                    if (!nw) {
                        errno = EINVAL;
                        snprintf(error, sizeof(error), FETT "Warning: " RESET "'" BLUE "%s" RESET "' not found [" MAGENTA "%d" RESET "]", tokens[i + 1].name, EOF_counter);
                        perror(error);
                    }
                }
                switch (current_section)
                {
                case 1:
                    jmp = append(jmp, formatted);
                    break;
                
                default:
                    code = append(code, formatted);
                    break;
                }
                break;
                
            case TOKEN_JMP:
                snprintf(formatted, sizeof(formatted), "jmp %s\n    ", tokens[i + 1].name);
                if (!is_in_array(jumps, jmp_count, tokens[i + 1].name)) {
                    if (!nw) {
                        errno = EINVAL;
                        snprintf(error, sizeof(error), FETT "Warning: " RESET "'" BLUE "%s" RESET "' not found [" MAGENTA "%d" RESET "]", tokens[i + 1].name, EOF_counter);
                        perror(error);
                    }
                }
                
                switch (current_section)
                {
                case 1:
                    jmp = append(jmp, formatted);
                    break;
                
                default:
                    code = append(code, formatted);
                    break;
                }
                break;
            case TOKEN_MCLAM:   
                expr = t.name;
                replace_all_vars(expr, variables, var_count);
                double value2 = te_interp(expr, 0);
                snprintf(t.name, sizeof(t.name), "%d", (int)value2);
                break;
            case TOKEN_COM:   break;
            case TOKEN_NUMBER:  break;
            case TOKEN_INT: 
                expr = t.name;
                replace_all_vars(expr, variables, var_count);
                value = te_interp(expr, 0);
                snprintf(t.name, sizeof(t.name), "%d", (int)value);
                t.value = (int)value;
                break;
            case TOKEN_VAR:  
                break;
            case TOKEN_EOF: EOF_counter++; break;
            case TOKEN_END: 
                switch (current_section)
                {
                case 1:
                    jmp = append(jmp, "jmp exit8\n    ");
                    break;
                
                default:
                    code = append(code, "jmp exit8\n    ");
                    break;
                }
                break;
                break;
            case TOKEN_TAB: break;
            case TOKEN_ECLAM: {
                char* rpush = malloc(1); 
                rpush[0] = '\0';
                char * push = strtok(t.name, ",");
                
                while(push != NULL) {
                    if (push[0] == '#') {
                        memmove(push, push + 1, strlen(push));
                        replace(push, '(', '['); replace(push, ')', ']');
                        snprintf(formatted, sizeof(formatted), "push dword %s\n    ", push);
                    } else {
                        snprintf(formatted, sizeof(formatted), "push %s\n    ", push);
                    }

                    rpush = append(rpush, formatted);
                    push = strtok(NULL, ",");
                }
                switch (current_section)
                {
                case 1:
                    jmp = append(jmp, rpush);
                    break;
                
                default:
                    code = append(code, rpush);
                    break;
                }
                break;
            }
            case TOKEN_POP: 
                if ((tokens[i + 1].type == TOKEN_EOF || tokens[i + 1].type == TOKEN_COM)) {
                    switch (current_section)
                    {
                    case 1:
                        jmp = append(jmp, "pop\n    ");
                        break;
                    
                    default:
                        code = append(code, "pop\n    ");
                        break;
                    }
                } else {
                    if (tokens[i + 1].dif == TOK_INT) {
                        expr = tokens[i + 1].name;
                        replace_all_vars(expr, variables, var_count);
                        value = te_interp(expr, 0);
                        snprintf(tokens[i + 1].name, sizeof(tokens[i + 1].name), "%d", (int)value);
                    }
                    if (tokens[i + 1].type == TOKEN_VAR) {
                        snprintf(formatted, sizeof(formatted), "push dword %s\n    ", tokens[i + 2].name);
                    } else {
                        snprintf(formatted, sizeof(formatted), "push %s\n    ", tokens[i + 1].name);
                    }
                    
                    if (is_in_array(variables, var_count, tokens[i + 1].name)) {
                        updateVar(variables, var_count, "eax", getValue(tokens[i + 1].name, variables, var_count), getChar(tokens[i + 1].name, variables, var_count));
                    } else {
                        updateVar(variables, var_count, "eax", atoi(tokens[i + 1].name), tokens[i + 1].name);
                    }
                    switch (current_section)
                    {
                    case 1:
                        jmp = append(jmp, formatted);
                        break;
                    
                    default:
                        code = append(code, formatted);
                        break;
                    }
                }
                break;
            case TOKEN_EQUAL:  
                if (!is_in_array(jumps, jmp_count, tokens[i + 1].name) && !nw) {
                    errno = EINVAL;
                    snprintf(error, sizeof(error), FETT "Warning: " RESET "'" BLUE "%s" RESET "' not found [" MAGENTA "%d" RESET "]", tokens[i + 1].name, EOF_counter);
                    perror(error);
                }
                snprintf(formatted, sizeof(formatted), "je %s\n    ", tokens[i + 1].name);
                switch (current_section)
                {
                case 1:
                    jmp = append(jmp, formatted);
                    break;
                
                default:
                    code = append(code, formatted);
                    break;
                }
                break;
            case TOKEN_RET:  
                switch (current_section)
                {
                case 1:
                    if (tokens[i + 2].type == TOKEN_ECLAM || tokens[i + 2].type == TOKEN_ARG) {
                        snprintf(formatted, sizeof(formatted), "mov eax, %s\n    ", tokens[i + 2].name);
                        jmp = append(jmp, formatted);
                    } if (tokens[i + 1].type == TOKEN_ECLAM || tokens[i + 1].type == TOKEN_ARG) {
                        snprintf(formatted, sizeof(formatted), "mov eax, %s\n    ret\n    ", tokens[i + 1].name);
                        jmp = append(jmp, formatted);
                    } else if (tokens[i + 1].type == TOKEN_NUMBER) {
                        snprintf(formatted, sizeof(formatted), "ret %d\n    ", tokens[i + 1].value);
                        jmp = append(jmp, formatted);
                    } else {
                        jmp = append(jmp, "ret\n    ");
                    }
                    break;
                
                default:
                    if (!nw) {
                        errno = EINVAL;
                        snprintf(error, sizeof(error), FETT "Warning: " RESET "Invalid use of return (Line: " MAGENTA "%d" RESET ")", EOF_counter);
                        perror(error);
                    }
                    break;
                }
                break;
            case TOKEN_ASM:
                snprintf(formatted, sizeof(formatted), "%s\n    ", t.name);
                if (tokens[i - 1].type == TOKEN_DEF) {data = append(data, formatted); break;} 
                else {
                    switch (current_section)
                    {
                    case 1:
                        jmp = append(jmp, formatted);
                        break;
                    
                    default:
                        code = append(code, formatted);
                        break;
                    }
                }
                break;
            case TOKEN_GRT:
                if (!is_in_array(jumps, jmp_count, tokens[i + 1].name) && !nw) {
                    errno = EINVAL;
                    snprintf(error, sizeof(error), FETT "Warning: " RESET "'" BLUE "%s" RESET "' not found [" MAGENTA "%d" RESET "]", tokens[i + 1].name, EOF_counter);
                    perror(error);
                }
                snprintf(formatted, sizeof(formatted), "jg %s\n    ", tokens[i + 1].name);
                switch (current_section)
                {
                case 1:
                    jmp = append(jmp, formatted);
                    break;
                
                default:
                    code = append(code, formatted);
                    break;
                }
                break;
            case TOKEN_LES: 
                if (!is_in_array(jumps, jmp_count, tokens[i + 1].name) && !nw) {
                    errno = EINVAL;
                    snprintf(error, sizeof(error), FETT "Warning: " RESET "'" BLUE "%s" RESET "' not found [" MAGENTA "%d" RESET "]", tokens[i + 1].name, EOF_counter);
                    perror(error);
                }
                snprintf(formatted, sizeof(formatted), "jl %s\n    ", tokens[i + 1].name);
                switch (current_section)
                {
                case 1:
                    jmp = append(jmp, formatted);
                    break;
                
                default:
                    code = append(code, formatted);
                    break;
                }
                break;
            case TOKEN_INC:  
                if (tokens[i - 2].type == TOKEN_VAR) {
                    snprintf(formatted, sizeof(formatted), "inc dword %s\n    ", tokens[i - 1].name);
                } else {
                    snprintf(formatted, sizeof(formatted), "inc %s\n    ", tokens[i - 1].name);
                }
                

                if (is_in_array(variables, var_count, tokens[i - 1].name)) {
                    updateVar(variables, var_count, tokens[i - 1].name, getValue(tokens[i - 1].name, variables, var_count) + 1, getChar(tokens[i - 1].name, variables, var_count));
                }

                switch (current_section)
                {
                case 1:
                    jmp = append(jmp, formatted);
                    break;
                
                default:
                    code = append(code, formatted);
                    break;
                }
                break;
            case TOKEN_NOT:  
                if (!is_in_array(jumps, jmp_count, tokens[i + 1].name) && !nw) {
                    errno = EINVAL;
                    snprintf(error, sizeof(error), FETT "Warning: " RESET "'" BLUE "%s" RESET "' not found [" MAGENTA "%d" RESET "]", tokens[i + 1].name, EOF_counter);
                    perror(error);
                }
                snprintf(formatted, sizeof(formatted), "jne %s\n    ", tokens[i + 1].name);
                switch (current_section)
                {
                case 1:
                    jmp = append(jmp, formatted);
                    break;
                
                default:
                    code = append(code, formatted);
                    break;
                }
                break;
            case TOKEN_SCMP:  
                if (i > 0 && (i + 1) < *token_count) {
                    Token left = tokens[i - 1];
                    Token right = tokens[i + 1];
                    if ((left.type == TOKEN_UNKNOWN && is_in_array(variables, var_count, left.name)) &&
                        (right.type == TOKEN_UNKNOWN && is_in_array(variables, var_count, right.name))) {
                        char cmp_code[512];
                        
                        snprintf(cmp_code, sizeof(cmp_code),
                                "push %s\n    push %s\n    call strcmp\n    add esp, 8\n    cmp eax, 0\n    ",
                                left.name, right.name);
                        
                        switch (current_section)
                        {
                        case 1:
                            jmp = append(jmp, cmp_code);
                            break;
                        
                        default:
                            code = append(code, cmp_code);
                            break;
                        }
                        i += 1;
                    } else {
                        if (left.type == TOKEN_UNKNOWN && is_in_array(variables, var_count, left.name)) {
                            errno = EINVAL;
                            snprintf(error, sizeof(error), MAGENTA "[%d]: " RESET "\033[1;31merror:" RESET FETT " Syntax error near string compare | " RESET RED "'" RESET BLUE "%s" RESET RED "' not found" RESET, EOF_counter, right.name);
                            perror(error);
                            return NULL;
                        } else {
                            errno = EINVAL;
                            snprintf(error, sizeof(error), MAGENTA "[%d]: " RESET "\033[1;31merror:" RESET FETT " Syntax error near string compare | " RESET RED "'" RESET BLUE "%s" RESET RED "' not found" RESET, EOF_counter, left.name);
                            perror(error);
                            return NULL;
                        }
                    }
                }
                break;
            case TOKEN_CMP: 
                if (i > 0 && (i + 1) < *token_count) {
                    Token left = tokens[i - 1];
                    Token right = tokens[i + 1];
                    char cmp_code[512];

                    if (tokens[i - 2].type == TOKEN_DEF) {
                        snprintf(cmp_code, sizeof(cmp_code),
                                "cmp %s, %s\n    ",
                                left.name, right.name);
                        switch (current_section)
                        {
                        case 1:
                            jmp = append(jmp, cmp_code);
                            break;
                        
                        default:
                            code = append(code, cmp_code);
                            break;
                        }
                        break;
                    }

                    if ((left.type == TOKEN_UNKNOWN && is_in_array(variables, var_count, left.name)) &&
                        (right.type == TOKEN_NUMBER || right.type == TOKEN_MCLAM || right.type == TOKEN_STR || (right.type == TOKEN_UNKNOWN && is_in_array(variables, var_count, right.name)))) {

                        if (right.type == TOKEN_NUMBER) {
                            snprintf(cmp_code, sizeof(cmp_code),
                                    "mov eax, [%s]\n    cmp eax, %d\n    ",
                                    left.name, right.value);
                        } else if (right.type == TOKEN_STR) {
                            snprintf(cmp_code, sizeof(cmp_code),
                                    "cmp %s, '%s'\n    ",
                                    left.name, right.name);
                        } else {
                            snprintf(cmp_code, sizeof(cmp_code),
                                    "mov eax, [%s]\n    cmp eax, [%s]\n    ",
                                    left.name, right.name);
                        } 

                        switch (current_section)
                        {
                        case 1:
                            jmp = append(jmp, cmp_code);
                            break;
                        
                        default:
                            code = append(code, cmp_code);
                            break;
                        }
                        i += 1;
                    } else {
                        if (left.type == TOKEN_UNKNOWN && is_in_array(variables, var_count, left.name)) {
                            errno = EINVAL;
                            snprintf(error, sizeof(error), MAGENTA "[%d]: " RESET "\033[1;31merror:" RESET FETT " Syntax error near compare | " RESET RED "'" RESET BLUE "%s" RESET RED "' not found" RESET, EOF_counter, right.name);
                            perror(error);
                            return NULL;
                        } else {
                            errno = EINVAL;
                            snprintf(error, sizeof(error), MAGENTA "[%d]: " RESET "\033[1;31merror:" RESET FETT " Syntax error near compare | " RESET RED "'" RESET BLUE "%s" RESET RED "' not found" RESET, EOF_counter, left.name);
                            perror(error);
                            return NULL;
                        }
                    }
                }
                break;
            case TOKEN_DEF: break;
            case TOKEN_STR: break;
            case TOKEN_ARG: break;
            case TOKEN_FLOAT: break;
            case TOKEN_MOV: 
                if (tokens[i - 2].type == TOKEN_VAR) {
                    snprintf(formatted, sizeof(formatted), "mov dword %s, %s\n    ", tokens[i - 1].name, tokens[i + 1].name);
                } else {
                    snprintf(formatted, sizeof(formatted), "mov %s, %s\n    ", tokens[i - 1].name, tokens[i + 1].name);
                }
                switch (current_section)
                {
                case 1:
                    jmp = append(jmp, formatted);
                    break;
                
                default:
                    code = append(code, formatted);
                    break;
                }
                break;
            case TOKEN_BSS: 
            	if (tokens[i + 2].type == TOKEN_INT) {
                    char* input = tokens[2 + i].name;
                    replace_all_vars(expr, variables, var_count);
                    value = te_interp(expr, 0);
                } else {
                    value = (tokens[i + 2].dif == TOK_INT) ? tokens[i + 2].value : atoi(tokens[i + 2].name);
                }
                
                if (tokens[i + 2].dif == TOK_INT) {
                    snprintf(formatted, sizeof(formatted), "%s resd %d\n    ", tokens[i + 1].name, tokens[i + 2].value);
                }  else {
                    snprintf(formatted, sizeof(formatted), "%s resb %s\n    ", tokens[i + 1].name, tokens[i + 2].name);
                }
                
                variables = addVar(variables, &var_count, tokens[i + 1].name, 0, "", TOK_UND);
                bss = append(bss, formatted);
                break;
            case TOKEN_CLAMP: break;
            case TOKEN_UNKNOWN:
                if ((tokens[i + 2].type == TOKEN_DEF)) {
                   if (strcmp(t.name, "dbl") == 0) {
                        variables = addVar(variables, &var_count, tokens[i+1].name, 0, tokens[i + 2].name, TOK_DOUBLE);
                        snprintf(formatted, sizeof(formatted), "%s dq %s, 0\n    ",
                                tokens[i + 1].name, tokens[i + 2].name);
                        data = append(data, formatted);

                    } else if (strcmp(t.name, "int") == 0) {
                        variables = addVar(variables, &var_count, tokens[i+1].name, 0, tokens[i + 2].name, TOK_INT);
                        snprintf(formatted, sizeof(formatted), "%s dd %s, 0\n    ",
                                tokens[i + 1].name, tokens[i + 2].name);
                        data = append(data, formatted);

                    } else if (strcmp(t.name, "str") == 0) {
                        variables = addVar(variables, &var_count, tokens[i+1].name, 0, tokens[i + 2].name, TOK_STR);
                        snprintf(formatted, sizeof(formatted), "%s db \"%s\", 0\n    ",
                                tokens[i + 1].name, tokens[i + 2].name);
                        data = append(data, formatted);

                    } else if (strcmp(t.name, "lng") == 0) {
                        variables = addVar(variables, &var_count, tokens[i+1].name, 0, tokens[i + 2].name, TOK_DOUBLE);
                        snprintf(formatted, sizeof(formatted), "%s dt %s, 0\n    ",
                                tokens[i + 1].name, tokens[i + 2].name);
                        data = append(data, formatted);

                    } else if (strcmp(t.name, "dwr") == 0) {
                        variables = addVar(variables, &var_count, tokens[i+1].name, 0, tokens[i + 2].name, TOK_DOUBLE);
                        snprintf(formatted, sizeof(formatted), "%s dw %s, 0\n    ",
                                tokens[i + 1].name, tokens[i + 2].name);
                        data = append(data, formatted);
                    }

                    
                }
                

                if (tokens[i + 1].type == TOKEN_DEF) { // Wenn es eine Definition
                    if (tokens[i + 2].type == TOKEN_JMP) { // Jump
                        snprintf(formatted, sizeof(formatted), "\n%s:\n    ", t.name);
                        jmp = append(jmp, formatted);
                        current_section = 1;
                        jumps = addVar(jumps, &jmp_count, t.name, 0, "", TOK_UND);
                        i += 2;
                    } else if (tokens[i + 2].dif == TOK_INT && !strcmp(tokens[i - 1].name, "dq")) {
                        char formatted[512];
                        snprintf(formatted, sizeof(formatted), "%s dq %s.%s\n    ", t.name, tokens[i + 2].name, tokens[i + 3].type == TOKEN_DEF ? tokens[i + 4].name : "0");
                        data = append(data, formatted);
                        variables = addVar(variables, &var_count, t.name, (int)value, "", TOK_DOUBLE);
                    } else if (tokens[i + 2].dif == TOK_INT) { // Definition eines Int.
                        double value;
                        char formatted[512];
                        if (tokens[i + 2].type == TOKEN_INT) {    
                            char* input = tokens[2 + i].name;
                            replace_all_vars(expr, variables, var_count);
                            value = te_interp(expr, 0);
                        } else {
                            value = tokens[i + 2].value;
                        }

                        if (!is_in_array(variables, var_count, t.name)) {
                            snprintf(formatted, sizeof(formatted), (floor(value) == value) ? "%s dd %.0f\n    " : "%s dq %f\n    ", t.name, value);
                            data = append(data, formatted);
                            variables = addVar(variables, &var_count, t.name, (int)value, "", (floor(value) == value) ? TOK_INT : TOK_DOUBLE);
                        } else {
                            if (floor(value) == value) {
                                snprintf(formatted, sizeof(formatted), "mov dword %s, %d\n    \n    ", t.name, (int)value);  
                                switch (current_section)
                                {
                                case 1:
                                    jmp = append(jmp, formatted);
                                    break;
                                
                                default:
                                    code = append(code, formatted);
                                    break;
                                }
                                updateVar(variables, var_count, t.name, (int)value, "");
                            }  
                        }
                        i += 2;

                    } else if (tokens[i + 2].dif == TOK_STR) {
                        char* string = tokens[i + 2].name;
                        int len = strlen(string);
                        char formatted[512];
                        if (!(len >= 2 && string[len - 1] == 'n' && string[len - 2] == '\\')) {
                            snprintf(formatted, sizeof(formatted), "%s db \"%s\", 0\n    ", t.name, string);
                        } else {
                            if (len >= 2) {
                                string[len - 2] = '\0';
                            }
                            snprintf(formatted, sizeof(formatted), "%s db \"%s\", 10, 0\n    ", t.name, string);
                        }
                        i += 2;
                        data = append(data, formatted);
                        variables = addVar(variables, &var_count, t.name, 0, string, TOK_STR);
                    } else if (tokens[i + 2].type == TOKEN_VAR) {
                        snprintf(formatted, sizeof(formatted), "%s db %s dup(0)\n    ", t.name, tokens[i + 3].name);
                        data = append(data, formatted);
                        variables = addVar(variables, &var_count, t.name, atoi(tokens[i + 3].name), tokens[i + 3].name, TOK_STR);
                    } else if (tokens[i + 2].type == TOKEN_ECLAM) {
                        snprintf(formatted, sizeof(formatted), "%s dd %s\n    ", t.name, tokens[i + 2].name);
                        data = append(data, formatted);
                        variables = addVar(variables, &var_count, t.name, 0, tokens[i + 2].name, TOK_UND);
                    }
                }
                /*  CMD/Built-ins  */
                else if (strcmp(t.name, "prompt") == 0) {
                    if (tokens[i + 1].type == TOKEN_UNKNOWN && is_in_array(variables, var_count, tokens[i + 2].name)) {
                        char formatted[512];
                        if (!is_in_array(variables, var_count, tokens[i + 1].name)) {
                            snprintf(formatted, sizeof(formatted), "%s db 128 dup(0)\n    ", tokens[i + 1].name);
                            data = append(data, formatted);
                            variables = addVar(variables, &var_count, tokens[i + 1].name, 0, "", TOK_STR);
                        }
#ifdef _WIN32
                            snprintf(formatted, sizeof(formatted), "push %s\n    call _printf\n    add esp, 4\n    push %s\n    call gets\n    add esp, 4\n    ", tokens[i + 2].name, tokens[i + 1].name);
#else
                            snprintf(formatted, sizeof(formatted), "push %s\n    call printf\n    add esp, 4\n    push %s\n    call fgets\n    add esp, 4\n    ", tokens[i + 2].name, tokens[i + 1].name);
#endif
                        
                        switch (current_section)
                        {
                        case 1:
                            jmp = append(jmp, formatted);
                            break;
                        
                        default:
                            code = append(code, formatted);
                            break;
                        }
                    } else {
                        errno = EINVAL;
                        snprintf(error, sizeof(error), MAGENTA "[%d]: " RESET "\033[1;31merror:" RESET FETT " Syntax error near promt" RESET, EOF_counter);
                        perror(error);
                        return NULL;
                    }
                } else if (!strcmp(t.name, "print")) {
                    char* string = malloc(1);
                    string[0] = '\0';
                    int current = 1;
                    int n = 0;
                    while (current < *token_count && tokens[i + current].type != TOKEN_EOF && tokens[i + current].type != TOKEN_COM) {
                        char formatted[512];
                        if (tokens[i + current].type == TOKEN_VAR) {
                            snprintf(formatted, sizeof(formatted), "push dword %s\n    ", tokens[i + current + 1].name);
                            current++;
                        } else {
                            if (!strcmp(tokens[i + current].name, "dq")) {
                                current++;
                                snprintf(formatted, sizeof(formatted), "push dword [%s+4]\n    push dword [%s]\n    ", tokens[i + current].name, tokens[i + current].name);
                            } else {
                                snprintf(formatted, sizeof(formatted), "push %s\n    ", tokens[i + current].name);
                            }
                        }
                        string = append(string, formatted);
                        n++;
                        current++;
                    }

                    char formatted[512];
                    snprintf(formatted, sizeof(formatted), "%scall _printf\n    add esp, %d\n    \n    ", string, (n) * 4);
#ifdef _WIN32
                        snprintf(formatted, sizeof(formatted), "%scall _printf\n    add esp, %d\n    \n    ", string, (n) * 4);
#else
                        snprintf(formatted, sizeof(formatted), "%scall printf\n    add esp, %d\n    \n    ", string, (n) * 4);
#endif

                    switch (current_section)
                    {
                    case 1:
                        jmp = append(jmp, formatted);
                        break;
                    
                    default:
                        code = append(code, formatted);
                        break;
                    }
                    i += n;
                } else if (!strcmp(t.name, "atoi")) {
                    if (is_in_array(variables, var_count, tokens[i + 2].name) && is_in_array(variables, var_count, tokens[i + 1].name)) {
#ifdef _WIN32
                            snprintf(formatted, sizeof(formatted), "push %s\n    call _atoi\n    add esp, 4\n    mov [%s], eax\n    ", tokens[i + 2].name, tokens[i + 1].name);
#else
                            snprintf(formatted, sizeof(formatted), "push %s\n    call atoi\n    add esp, 4\n    mov [%s], eax\n    ", tokens[i + 2].name, tokens[i + 1].name);
#endif
                        updateVar(variables, var_count, tokens[i + 1].name, atoi(getChar(tokens[i + 2].name, variables, var_count)), tokens[i + 1].name);
                        switch (current_section)
                        {
                        case 1:
                            jmp = append(jmp, formatted);
                            break;
                        
                        default:
                            code = append(code, formatted);
                            break;
                        }
                    } else {
                        if (is_in_array(variables, var_count, tokens[i + 2].name)) {
                            errno = EINVAL;
                            snprintf(error, sizeof(error), MAGENTA "%d:" RESET  RED"Error: Syntax error near atoi | " RESET RED "'" RESET BLUE "%s" RESET RED "' not found" RESET, EOF_counter, tokens[i + 1].name);
                            perror(error);
                            return NULL;
                        } else {
                            errno = EINVAL;
                            snprintf(error, sizeof(error), MAGENTA "%d:" RESET  RED"Error: Syntax error near atoi | " RESET RED "'" RESET BLUE "%s" RESET RED "' not found" RESET, EOF_counter, tokens[i + 2].name);
                            perror(error);
                            return NULL;
                        }
                    }
                } else if (!strcmp(t.name, "include")) {
                    // include things to .text
                    
                    if (tokens[i + 1].type == TOKEN_ECLAM) {
                        snprintf(ic, sizeof(ic), "%s.obj ", tokens[i + 1].name);   
                        *incl = append(*incl, ic);
                    }

                    const char *fn = strrchr(tokens[i + 1].name, '\\');
                    if (fn) {
                        fn++;
                        snprintf(formatted, sizeof(formatted), ", %s", fn);
                    } else {
                        snprintf(formatted, sizeof(formatted), ", %s", tokens[i + 1].name);
                    }
                    text = append(text, formatted);
                    jumps = addVar(jumps, &jmp_count, tokens[i + 1].name, 0, "", TOK_UND);
                    i++;

                } else if (!strcmp(t.name, "clr")) {
                    if (tokens[i + 1].type == TOKEN_NUMBER) {
                        snprintf(formatted, sizeof(formatted), "add esp, %d\n    ", tokens[i + 1].value);
                        switch (current_section)
                        {
                        case 1:
                            jmp = append(jmp, formatted);
                            break;
                        
                        default:
                            code = append(code, formatted);
                            break;
                        }
                    } else {
                        errno = EINVAL;
                        snprintf(error, sizeof(error), MAGENTA "%d:" RESET  RED"Error: Syntax error near clear | No number to push" RESET, EOF_counter);
                        perror(error);
                        return NULL;
                    }
                } else if (!strcmp(t.name, "randint")) {
                    if (tokens[i - 1].type == TOKEN_UNKNOWN && is_in_array(variables, var_count, tokens[i - 1].name)) {
                        if ((tokens[i + 1].type == TOKEN_NUMBER && tokens[i + 2].type == TOKEN_NUMBER)) {
                            if (ri) {
                                snprintf(formatted, sizeof(formatted), "push %d\n    push %d\n    call randint\n    add esp, 8\n    mov [%s], eax\n    ", tokens[i + 2].value, tokens[i + 1].value, tokens[i - 1].name);
                            } else {
                                int rd_num = rand() % (tokens[i + 2].value - tokens[i + 1].value + 1) + tokens[i + 1].value;
                                snprintf(formatted, sizeof(formatted), "mov dword %s, %d\n    ", tokens[i - 1].name, rd_num);
                            }
                        } else if (is_in_array(variables, var_count, tokens[i + 1].name) && is_in_array(variables, var_count, tokens[i + 2].name)) {
                            if (ri) {
                                snprintf(formatted, sizeof(formatted), "push %d\n    push %d\n    call randint\n    add esp, 8\n    mov [%s], eax\n    ", getValue(tokens[i + 2].name, variables, var_count), getValue(tokens[i + 1].name, variables, var_count), tokens[i - 1].name);
                            } else {
                                int rd_num = rand() % (getValue(tokens[i + 2].name, variables, var_count) - getValue(tokens[i + 1].name, variables, var_count) + 1) + getValue(tokens[i + 1].name, variables, var_count);
                                snprintf(formatted, sizeof(formatted), "mov dword %s, %d\n    ", tokens[i - 1].name, rd_num);
                            }
                        } else {
                            errno = EINVAL;
                            snprintf(error, sizeof(error), MAGENTA "%d:" RESET  RED"Error: Syntax error near random int. | No number to push" RESET, EOF_counter);
                            perror(error);
                            return NULL;
                        }   
                        switch (current_section)
                        {
                        case 1:
                            jmp = append(jmp, formatted);
                            break;
                        
                        default:
                            code = append(code, formatted);
                            break;
                        }   
                    } else {
                        errno = EINVAL;
                        snprintf(error, sizeof(error), MAGENTA "%d:" RESET  RED"Error: Syntax error near random int. | Missing var to load" RESET, EOF_counter);
                        perror(error);
                        return NULL;
                    }
                } else if (!strcmp(t.name, "define")) {
                    snprintf(formatted, sizeof(formatted), "%%define %s %s\n", tokens[i + 1].name, tokens[i + 2].name);
                    df = append(df, formatted);
                }
                // ASM
                else if (!strcmp(t.name, "mov")) {
                    if (tokens[i + 1].type == TOKEN_VAR) {
                        snprintf(formatted, sizeof(formatted), "mov dword %s, %s\n    ", tokens[i + 2].name, tokens[i + 3].name);
                    } else {
                        snprintf(formatted, sizeof(formatted), "mov %s, %s\n    ", tokens[i + 1].name, tokens[i + 2].name);
                    }
                    switch (current_section)
                    {
                    case 1:
                        jmp = append(jmp, formatted);
                        break;
                    
                    default:
                        code = append(code, formatted);
                        break;
                    } 
                } else if (!strcmp(t.name, "add")) {
                    if (!strcmp(tokens[i - 1].name, "dq")) {
                        if (tokens[i + 3].type == TOKEN_DEF) {
                            snprintf(formatted, sizeof(formatted), "movsd xmm0, %s\n    movsd xmm1, %s\n    addsd xmm0, xmm1\n    movsd %s, xmm0\n    ", tokens[i + 1].name, tokens[i + 2].name, tokens[i + 4].name);
                        } else {
                            snprintf(formatted, sizeof(formatted), "movsd xmm0, %s\n    movsd xmm1, %s\n    addsd xmm0, xmm1\n    ", tokens[i + 1].name, tokens[i + 2].name);
                        }
                    } else {
                        if (tokens[i + 3].type == TOKEN_DEF) {
                            snprintf(formatted, sizeof(formatted), "mov eax, %s\n    add eax, %s\n    mov %s, eax\n    ", tokens[i + 1].name, tokens[i + 2].name, tokens[i + 4].name);
                        } else {
                            snprintf(formatted, sizeof(formatted), "mov eax, %s\n    add eax, %s\n    ", tokens[i + 1].name, tokens[i + 2].name);
                        }   
                    }

                    switch (current_section)
                    {
                    case 1:
                        jmp = append(jmp, formatted);
                        break;
                    default:
                        code = append(code, formatted);
                        break;
                    }
                    i += 3;
                } else if (!strcmp(t.name, "math")) {
                    int current = i + 1;
                    char* math = malloc(1); 
                    math[0] = '\0';

                    while (!(tokens[current].type == TOKEN_EOF || tokens[current].type == TOKEN_COM)) {
                        if (tokens[current].type == TOKEN_SUB) {
                            if (current - 2 == i) {
                                snprintf(formatted, sizeof(formatted),
                                        "mov eax, %s\n    sub eax, %s\n    ",
                                        tokens[current - 1].name, tokens[current + 1].name);
                            } else {
                                snprintf(formatted, sizeof(formatted),
                                        "sub eax, %s\n    ",
                                        tokens[current + 1].name);
                            }
                            math = append(math, formatted);

                        } else if (tokens[current].type == TOKEN_END) {   // multiplication
                            if (current - 2 == i) {
                                snprintf(formatted, sizeof(formatted),
                                        "mov eax, %s\n    mov ebx, %s\n    mul ebx\n    ",
                                        tokens[current - 1].name, tokens[current + 1].name);
                            } else {
                                snprintf(formatted, sizeof(formatted),
                                        "mov ebx, %s\n    mul ebx\n    ",
                                        tokens[current + 1].name);
                            }
                            math = append(math, formatted);

                        } else if (tokens[current].type == TOKEN_RET) {   // division
                            if (current - 2 == i) {
                                snprintf(formatted, sizeof(formatted),
                                        "mov eax, %s\n    mov ebx, %s\n    xor edx, edx\n    div ebx\n    ",
                                        tokens[current - 1].name, tokens[current + 1].name);
                            } else {
                                snprintf(formatted, sizeof(formatted),
                                        "mov ebx, %s\n    xor edx, edx\n    div ebx\n    ",
                                        tokens[current + 1].name);
                            }
                            math = append(math, formatted);

                        } else if (tokens[current].type == TOKEN_POP) {   // addition
                            if (current - 2 == i) {
                                snprintf(formatted, sizeof(formatted),
                                        "mov eax, %s\n    add eax, %s\n    ",
                                        tokens[current - 1].name, tokens[current + 1].name);
                            } else {
                                snprintf(formatted, sizeof(formatted),
                                        "add eax, %s\n    ",
                                        tokens[current + 1].name);
                            }
                            math = append(math, formatted);
                        }

                        current++;
                    }
                    i = current;
                    switch (current_section)
                    {
                    case 1:
                        jmp = append(jmp, math);
                        break;
                    default:
                        code = append(code, math);
                        break;
                    }
                } else if (!strcmp(t.name, "sub")) {
                    if (!strcmp(tokens[i - 1].name, "dq")) {
                        if (tokens[i + 3].type == TOKEN_DEF) {
                            snprintf(formatted, sizeof(formatted), "movsd xmm0, %s\n    movsd xmm1, %s\n    subsd xmm0, xmm1\n    movsd %s, xmm0\n    ", tokens[i + 1].name, tokens[i + 2].name, tokens[i + 4].name);
                        } else {
                            snprintf(formatted, sizeof(formatted), "movsd xmm0, %s\n    movsd xmm1, %s\n    subsd xmm0, xmm1\n    ", tokens[i + 1].name, tokens[i + 2].name);
                        }
                    } else {
                        if (tokens[i + 3].type == TOKEN_DEF) {
                            snprintf(formatted, sizeof(formatted), "mov eax, %s\n    sub eax, %s\n    mov %s, eax\n    ", tokens[i + 1].name, tokens[i + 2].name, tokens[i + 4].name);
                        } else {
                            snprintf(formatted, sizeof(formatted), "mov eax, %s\n    sub eax, %s\n    ", tokens[i + 1].name, tokens[i + 2].name);
                        }
                    }
                    
                    switch (current_section)
                    {
                    case 1:
                        jmp = append(jmp, formatted);
                        break;
                    default:
                        code = append(code, formatted);
                        break;
                    }
                    i += 3;
                }  else if (!strcmp(t.name, "mul")) {
                    if (!strcmp(tokens[i - 1].name, "dq")) {
                        if (tokens[i + 3].type == TOKEN_DEF) {
                            snprintf(formatted, sizeof(formatted), "movsd xmm0, %s\n    movsd xmm1, %s\n    mulsd xmm0, xmm1\n    movsd %s, xmm0\n    ", tokens[i + 1].name, tokens[i + 2].name, tokens[i + 4].name);
                        } else {
                            snprintf(formatted, sizeof(formatted), "movsd xmm0, %s\n    movsd xmm1, %s\n    mulsd xmm0, xmm1\n    ", tokens[i + 1].name, tokens[i + 2].name);
                        }
                    } else {
                        if (tokens[i + 3].type == TOKEN_DEF) {
                            snprintf(formatted, sizeof(formatted), "mov eax, %s\n    mov ebx, %s\n    mul ebx\n    mov %s, eax\n    ", tokens[i + 1].name, tokens[i + 2].name, tokens[i + 4].name);
                        } else {
                            snprintf(formatted, sizeof(formatted), "mov eax, %s\n    mov ebx, %s\n    mul ebx\n    ", tokens[i + 1].name, tokens[i + 2].name);
                        }
                    }
                    
                    switch (current_section)
                    {
                    case 1:
                        jmp = append(jmp, formatted);
                        break;
                    default:
                        code = append(code, formatted);
                        break;
                    }
                    i += 3;
                } else if (!strcmp(t.name, "div")) {
                    if (!strcmp(tokens[i - 1].name, "dq")) {
                        if (tokens[i + 3].type == TOKEN_DEF) {
                            snprintf(formatted, sizeof(formatted), "movsd xmm0, %s\n    movsd xmm1, %s\n    divsd xmm0, xmm1\n    movsd %s, xmm0\n    ", tokens[i + 1].name, tokens[i + 2].name, tokens[i + 4].name);
                        } else {
                            snprintf(formatted, sizeof(formatted), "movsd xmm0, %s\n    movsd xmm1, %s\n    divsd xmm0, xmm1\n    ", tokens[i + 1].name, tokens[i + 2].name);
                        }
                    } else {
                        if (tokens[i + 3].type == TOKEN_DEF) {
                            snprintf(formatted, sizeof(formatted), "mov eax, %s\n    mov ebx, %s\n    xor edx, edx\n    div ebx\n    mov %s, eax\n    ", tokens[i + 1].name, tokens[i + 2].name, tokens[i + 4].name);
                        } else {
                            snprintf(formatted, sizeof(formatted), "mov eax, %s\n    mov ebx, %s\n    xor edx, edx\n    div ebx\n    ", tokens[i + 1].name, tokens[i + 2].name);
                        }
                    }
                    
                    switch (current_section)
                    {
                    case 1:
                        jmp = append(jmp, formatted);
                        break;
                    default:
                        code = append(code, formatted);
                        break;
                    }
                    i += 3;
                } else if (!strcmp(t.name, "xor")) {
                    if (tokens[i + 1].type == TOKEN_VAR) {
                        snprintf(formatted, sizeof(formatted), "xor dword %s, %s\n    ", tokens[i + 2].name, tokens[i + 3].name);
                    } else {
                        snprintf(formatted, sizeof(formatted), "xor %s, %s\n    ", tokens[i + 1].name, tokens[i + 2].name);
                    }
                    switch (current_section)
                    {
                    case 1:
                        jmp = append(jmp, formatted);
                        break;
                    
                    default:
                        code = append(code, formatted);
                        break;
                    } 
                } else if (!strcmp(t.name, "try")) {
                    if (tokens[i + 1].type == TOKEN_VAR) {
                        snprintf(formatted, sizeof(formatted), "test dword %s, %s\n    ", tokens[i + 2].name, tokens[i + 3].name);
                    } else {
                        snprintf(formatted, sizeof(formatted), "test %s, %s\n    ", tokens[i + 1].name, tokens[i + 2].name);
                    }
                    switch (current_section)
                    {
                    case 1:
                        jmp = append(jmp, formatted);
                        break;
                    
                    default:
                        code = append(code, formatted);
                        break;
                    } 
                }
                break;
            default:
                if (!nw) {
                    printf(FETT "Warning: " RESET "Unhandled token\n");
                }
        }
    }
    code = append(code, "jmp exit8\n\n");
#ifdef _WIN32
        jmp = append(jmp, "\nexit8:\n    push 0\n    call _ExitProcess@4\n");
#else
        jmp = append(jmp, "\nexit8:\n    push 0\n    call exit\n");
#endif

    data = append(data, "\n");
    df = append(df, "\n");
#ifdef _WIN32
        text = append(text, ", _ExitProcess@4\n\n");
#else
        text = append(text, ", exit\n\n");
#endif

    char* output = malloc(1);
    output[0] = '\0';
    if (bssb) {
        bss = append(bss, "\n");
        output = append(output, bss);
    }
    output = append(output, text);
    output = append(output, df);
    output = append(output, code);
    output = append(output, jmp);

    data = append(data, output);
    free(output);
    free(variables);

    return data;
}

void generate_random_string(char *str, size_t length) {
    const char charset[] = "0123456789abcdef"; // Hex
    for (size_t i = 0; i < length; i++) {
        int key = rand() % 16;
        str[i] = charset[key];
    }
    str[length] = '\0';
}

int file_exists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1; // Datei existiert
    }
    return 0; // Datei existiert nicht
}

double get_time_sec() {
#ifdef _WIN32
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)freq.QuadPart;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
#endif
}

// Main code
int main(int argc, char *argv[]) {
    bool tok = false; // Prints tokens
    bool noWarning = false;
    bool a = false;
    bool o = false;
    bool ri = false;
    bool bss = false;
    bool opt = false;
    bool info = false;
    bool noconsole = false;
    int total_token_count = 0; // Total token count
    Token *all_tokens = malloc(sizeof(Token) * 10); // All the tokens
    int all_tokens_capacity = 10; // Total token capacity
    char error[256];
    char asmFile[512] = {0};
    char exeFile[512] = {0};
    char objFile[512] = {0};
    char output_redirect[128] = {0};
    double start_time = 0;

    /* FILENAME */
    const char *filename = NULL; 
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-tok") == 0) {
            tok = true;
        } else if (strcmp(argv[i], "-nw") == 0) {
            noWarning = true;
        } else if (strcmp(argv[i], "-asm") == 0) {
            a = true;
        } else if (strcmp(argv[i], "-o") == 0) {
            o = true;
        } else if (strcmp(argv[i], "-nc") == 0) {
            noconsole = true;
        } else if (strcmp(argv[i], "-gop") == 0) {
            opt = true;
        } else if (strcmp(argv[i], "-in") == 0) {
            info = true;
        } else if (argv[i][0] != '-') {
            filename = argv[i];
        }
    }

    /* Help if filename was not found */
    if (!filename) {
        fprintf(stderr, "Usage: %s <inputfile> [-tok] [-asm] [-nw] [-o] [-nc] [-gop] [-in]\n", argv[0]);
        goto cleanup;
    }

    if (info) {start_time = get_time_sec();}

    /* OPEN FILE */
    size_t len = strlen(filename);
    FILE *file = fopen(filename, "r");
    if (!file) {
        errno = ENOENT;
        snprintf(error, sizeof(error), "Error: Failed to open file | '%s'", filename);
        perror(error);
        goto cleanup;
    } 

    /* Read file into lines */
    char *inputs[MAX_LINES];
    int count = 0;
    char buffer[MAX_LINE_LEN];

    while (fgets(buffer, sizeof(buffer), file) && count < MAX_LINES) {
        buffer[strcspn(buffer, "\n")] = 0; // remove newline
        inputs[count] = strdup(buffer);
        count++;
    }
    fclose(file);
    file = NULL;

    /* tokenisieren */
    int input_count = count;
    for (int i = 0; i < input_count; i++) {
        int token_count = 0;
        Token *tokens = inter(inputs[i], &token_count, tok, &ri, &bss);
        if (!tokens) continue;

        if (total_token_count + token_count > all_tokens_capacity) {
            all_tokens_capacity *= 2;
            all_tokens = realloc(all_tokens, sizeof(Token) * all_tokens_capacity);
            if (!all_tokens) {
                fprintf(stderr, "Memory allocation failed\n");
                free(tokens);
                goto cleanup;
            }
        }

        for (int j = 0; j < token_count; j++) {
            all_tokens[total_token_count++] = tokens[j];
        }
        free(tokens);
    }

    /* parser/emit */
    char* incl = malloc(1); 
    if (!incl) goto cleanup;
    incl[0] = '\0'; 
    char* code = parser(all_tokens, &total_token_count, &incl, noWarning, ri, bss); // NASM-code

    if (code == NULL) { goto cleanup; }

    /* output redirect string */
#ifdef _WIN32
    if (opt) {
        snprintf(output_redirect, sizeof(output_redirect), "%s", "");
    } else {
        snprintf(output_redirect, sizeof(output_redirect), "%s", ">nul 2>&1");
    }
#else
    if (opt) {
        snprintf(output_redirect, sizeof(output_redirect), "%s", "");
    } else {
        snprintf(output_redirect, sizeof(output_redirect), "%s", "> /dev/null 2>&1");
    }
#endif

    /* --- Option: write .asm file and exit --- */
    if (a) {
        char formatted[512];
        if (len >= 4 && strcmp(filename + len - 4, ".qwr") == 0) {
            strncpy(formatted, filename, len - 4);
            formatted[len - 4] = '\0';
        } else {
            strncpy(formatted, filename, sizeof(formatted) - 1);
            formatted[sizeof(formatted) - 1] = '\0';
        }

        strncat(formatted, ".asm", sizeof(formatted) - strlen(formatted) - 1);
        FILE *datei = fopen(formatted, "w");
        if (!datei) {
            errno = EINVAL;
            perror("Error: File was not created");
            goto cleanup;
        }
        fputs(code, datei);
        fclose(datei);
    }
    /* --- Option: build to specific EXE name (-o) --- */
    else if (o) {
        char name[512];
        char asm_out[512];
        char exe_out[512];
        char obj_out[512];

        if (len >= 4 && strcmp(filename + len - 4, ".qwr") == 0) {
            strncpy(name, filename, len - 4);
            name[len - 4] = '\0';
        } else {
            errno = ENOENT;
            perror("Error: Failed to create EXE");
            goto cleanup;
        }

#ifdef _WIN32
        snprintf(asm_out, sizeof(asm_out), "%s.asm", name);
        snprintf(exe_out, sizeof(exe_out), "%s.exe", name);
        snprintf(obj_out, sizeof(obj_out), "%s.obj", name);
#else
        snprintf(asm_out, sizeof(asm_out), "%s.asm", name);
        snprintf(exe_out, sizeof(exe_out), "%s", name);
        snprintf(obj_out, sizeof(obj_out), "%s.o", name);
#endif

        FILE *asmf = fopen(asm_out, "w");
        if (!asmf) { perror("Error creating asm file"); goto cleanup; }
        fputs(code, asmf);
        fclose(asmf);

        char nasmCmd[2048];
        char linkCmd[2048];

#ifdef _WIN32
        snprintf(nasmCmd, sizeof(nasmCmd), "nasm -f win32 \"%s\" -o \"%s\"", asm_out, obj_out);
        if (noconsole) {
            snprintf(linkCmd, sizeof(linkCmd),
                "gcc -m32 -nostartfiles -Wl,-e,_start -Wl,-subsystem,console "
                "\"%s\" -o \"%s\" %s -lkernel32 -luser32 -lmsvcrt %s",
                obj_out, exe_out, incl, output_redirect);
        } else {
            snprintf(linkCmd, sizeof(linkCmd),
                "gcc -m32 -nostartfiles -Wl,-e,_start -Wl,-subsystem,windows "
                "\"%s\" -o \"%s\" %s -lkernel32 -luser32 -lmsvcrt %s",
                obj_out, exe_out, incl, output_redirect);
        }
#else
        snprintf(nasmCmd, sizeof(nasmCmd), "nasm -f elf32 %s -o %s", asm_out, obj_out);
        /* Use gcc to link so libc is linked automatically; -m32 assumes 32-bit object */
        if (noconsole) {
            snprintf(linkCmd, sizeof(linkCmd), "gcc -m32 %s -o %s %s", obj_out, exe_out, output_redirect);
        } else {
            snprintf(linkCmd, sizeof(linkCmd), "gcc -m32 \"%s\" -o %s %s", obj_out, exe_out, output_redirect);
        }
#endif
        
        if (system(nasmCmd) != 0) {
            fprintf(stderr, "Error: NASM assembly failed\n");
            goto cleanup;
        }

        if (system(linkCmd) != 0) {
            printf("[LINK CMD]\n%s\n", linkCmd);
            goto cleanup;
        }

        if (info) {
            double end_time = get_time_sec();
            printf("\n----------[ FINISH ]---------\nCompiling took %.2f seconds.\n", end_time - start_time);
        }

        /* cleanup generated intermediate files */
        if (file_exists(obj_out)) remove(obj_out);
        if (file_exists(asm_out)) remove(asm_out);
    }
    /* --- Default: create temp files, assemble, link, run, cleanup --- */
    else {
        srand((unsigned int) time(NULL));
        char uuid_str[33];  /* 32 chars + NUL */
        generate_random_string(uuid_str, 32);

#ifdef _WIN32
        const char *tmpdir = getenv("TEMP");
        if (!tmpdir) tmpdir = ".";
        snprintf(objFile, sizeof(objFile), "%s\\%s.obj", tmpdir, uuid_str);
        snprintf(asmFile, sizeof(asmFile), "%s\\%s.asm", tmpdir, uuid_str);
        snprintf(exeFile, sizeof(exeFile), "%s\\%s.exe", tmpdir, uuid_str);
#else
        const char *tmpdir = getenv("TMPDIR");
        if (!tmpdir) tmpdir = "/tmp";
        snprintf(objFile, sizeof(objFile), "%s/%s.o", tmpdir, uuid_str);
        snprintf(asmFile, sizeof(asmFile), "%s/%s.asm", tmpdir, uuid_str);
        snprintf(exeFile, sizeof(exeFile), "%s/%s", tmpdir, uuid_str);
#endif

        FILE *asmf = fopen(asmFile, "w");
        if (!asmf) { perror("Error creating temporary asm file"); goto cleanup; }
        fputs(code, asmf);
        fclose(asmf);

        char nasmCmd[2048];
        char linkCmd[2048];

#ifdef _WIN32
        snprintf(nasmCmd, sizeof(nasmCmd), "nasm -f win32 %s -o %s", asmFile, objFile);
        if (noconsole) {
            snprintf(linkCmd, sizeof(linkCmd),
                "gcc -m32 -nostartfiles -Wl,-e,_start -Wl,-subsystem,windows "
                "%s -o %s %s -lkernel32 -luser32 -lmsvcrt %s",
                objFile, exeFile, incl, output_redirect);
        } else {
            snprintf(linkCmd, sizeof(linkCmd),
                "gcc -m32 -nostartfiles -Wl,-e,_start -Wl,-subsystem,windows "
                "%s -o %s %s -lkernel32 -luser32 -lmsvcrt %s",
                objFile, exeFile, incl, output_redirect);
        }
#else
        snprintf(nasmCmd, sizeof(nasmCmd), "nasm -f elf32 %s -o %s", asmFile, objFile);
        snprintf(linkCmd, sizeof(linkCmd), "gcc -m32 %s -o %s %s", objFile, exeFile, output_redirect);
#endif

        if (system(nasmCmd) != 0) {
            fprintf(stderr, "Error: NASM assembly failed\n");
            goto cleanup;
        }

        if (system(linkCmd) != 0) {
            printf("[LINK CMD]\n%s\n", linkCmd);
            goto cleanup;
        }

        if (info) {
            double end_time = get_time_sec();
            printf("\n----------[ FINISH ]---------\nCompiling took %.2f seconds.\n", end_time - start_time);
        }

        /* run the program */
#ifdef _WIN32
        /* Windows: system() can run the .exe directly */
        if (system(exeFile) != 0) {
            /* Non-zero return is allowed; don't abort cleanup */
        }
#else
        /* Make sure executable bit is set */
        chmod(exeFile, 0755);
        if (system(exeFile) != 0) {
            /* Non-zero return is allowed; don't abort cleanup */
        }
#endif

        /* Clear temporary files */
        if (file_exists(asmFile)) remove(asmFile);
        if (file_exists(objFile)) remove(objFile);
        if (file_exists(exeFile)) remove(exeFile);
    }

cleanup:
    /* Try to remove any leftover files (best-effort) */
    if (asmFile[0]) { if (file_exists(asmFile)) remove(asmFile); }
    if (objFile[0]) { if (file_exists(objFile)) remove(objFile); }
    if (exeFile[0]) { if (file_exists(exeFile)) remove(exeFile); }

    if (all_tokens) free(all_tokens);
    if (inputs) {
        for (int i = 0; i < count; i++) {
            if (inputs[i]) free(inputs[i]);
        }
    }
    if (code) free(code);
    if (file) fclose(file);
    if (incl) free(incl);

    return 0;
}