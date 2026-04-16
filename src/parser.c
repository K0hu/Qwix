// parser.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include "tokens.h"

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

enum Type {
    TYPE_BYTE,
    TYPE_SHORT,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_DOUBLE
};

typedef struct {
    char name[32];
    int offset; // stack offset
    enum Type type; // 1: byte, 2: short, 4: int, 5: float, 8: double
} Var;

int find_var_by_name(const char *name, Var *vars, int var_count) {
    for (int i = 0; i < var_count; i++)
    {
        if (strcmp(vars[i].name, name) == 0)
        {
            return i; 
        }
    }
    return -1; 
}

typedef struct {
    Token *tok;
    char code[1024];
} Parser;

void load(char *l, size_t size, Token *tok, Var *vars, int var_count) {
    if (tok->type == TOKEN_ID) {
        // Save variable
        int var_index = find_var_by_name(tok->name, vars, var_count);
        if (var_index < 0) return;
        switch (vars[var_index].type) {
            case TYPE_BYTE:
                snprintf(l, size,
                    "mov al, [rbp-%d]",
                    vars[var_index].offset);
                break;

            case TYPE_SHORT:
                snprintf(l, size,
                    "mov ax, [rbp-%d]",
                    vars[var_index].offset);
                break;

            case TYPE_INT:
                snprintf(l, size,
                    "mov eax, [rbp-%d]",
                    vars[var_index].offset);
                break;

            case TYPE_DOUBLE:
                snprintf(l, size,
                    "mov rax, [rbp-%d]",
                    vars[var_index].offset);
                break;
            default:
                break;
        }
    }
}

void save(char *s, size_t size, Token *tok, Var *vars, int var_count) {
    if (tok->type == TOKEN_ID) {
        // Save variable
        int var_index = find_var_by_name(tok->name, vars, var_count);
        if (var_index < 0) return;
        switch (vars[var_index].type) {
            case TYPE_BYTE:
                snprintf(s, size,
                    "mov BYTE PTR [rbp-%d], al",
                    vars[var_index].offset);
                break;

            case TYPE_SHORT:
                snprintf(s, size,
                    "mov WORD PTR [rbp-%d], ax",
                    vars[var_index].offset);
                break;

            case TYPE_INT:
                snprintf(s, size,
                    "mov DWORD PTR [rbp-%d], eax",
                    vars[var_index].offset);
                break;

            case TYPE_DOUBLE:
                snprintf(s, size,
                    "mov QWORD PTR [rbp-%d], rax",
                    vars[var_index].offset);
                break;
            default:
                break;
        }
    }
}

Parser parse(Token *tokens, int count) {

    // Search
    Var vars[1024];
    int var_count = 0;
    int offset = 0;
    for (int i = 0; i < count; i++) {
        switch (tokens[i].type)
        {
        case TOKEN_BYTE:
            strncpy(vars[var_count].name, tokens[i + 1].name, sizeof(vars[var_count].name) - 1);
            offset += 1;
            vars[var_count].offset = offset;
            vars[var_count].type = TYPE_BYTE;
            var_count++;
            break;
        
        case TOKEN_SHORT:
            strncpy(vars[var_count].name, tokens[i + 1].name, sizeof(vars[var_count].name) - 1);
            offset += 2;
            vars[var_count].offset = offset;
            vars[var_count].type = TYPE_SHORT;
            var_count++;
            break;
        
        case TOKEN_INT:
            strncpy(vars[var_count].name, tokens[i + 1].name, sizeof(vars[var_count].name) - 1);
            offset += 4;
            vars[var_count].offset = offset;
            vars[var_count].type = TYPE_INT;
            var_count++;
            break;
        
        case TOKEN_FLOAT:
            strncpy(vars[var_count].name, tokens[i + 1].name, sizeof(vars[var_count].name) - 1);
            offset += 4;
            vars[var_count].offset = offset;
            vars[var_count].type = TYPE_FLOAT;
            var_count++;
            break;
        
        case TOKEN_DOUBLE:
            strncpy(vars[var_count].name, tokens[i + 1].name, sizeof(vars[var_count].name) - 1);
            offset += 8;
            vars[var_count].offset = offset;
            vars[var_count].type = TYPE_DOUBLE;
            var_count++;
            break;

        default:
            break;
        }
    }

    int level = 0;
    Parser code;
    code.tok = tokens;
    for (int i = 0; i < count; i++) {
        switch (code.tok[i].type) {

            // Move
            case TOKEN_ARROWR: 
                Token left = code.tok[i - 1];
                Token right = code.tok[i + 1];

                if (left.type == TOKEN_ID) {
                    // Load variable
                    snprintf(code.code, sizeof(code.code), "mov rax, [%s]", left.name);
                } else if (left.type == TOKEN_NUMBER) {
                    // Load immediate value
                    snprintf(code.code, sizeof(code.code), "mov rax, %lf", left.value);
                }

                snprintf(code.code, sizeof(code.code), "mov ");
                break;

            case TOKEN_ARROWL: 
                Token left = code.tok[i - 1];
                Token right = code.tok[i + 1];

                snprintf(code.code, sizeof(code.code), "mov ");
                break;

            default:
                break;
        }
    }
}
