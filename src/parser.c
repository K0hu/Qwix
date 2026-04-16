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

typedef struct {
    char name[32];
    int index;
} Var;

typedef struct {
    Token *tok;
    char code[1024];
} Parser;

char load(Token *tok) {
    
}

char save(Token *tok) {

}

Parser parse(Token *tokens, int count) {

    // Search
    Var vars[1024];
    int var_count = 0;
    for (int i = 0; i < count; i++) {
        switch (tokens[i].type)
        {
        case TOKEN_BYTE:
            strncpy(vars[var_count].name, tokens[i + 1].name, sizeof(vars[var_count].name) - 1);
            vars[var_count].index = 1;
            var_count++;
        
        case TOKEN_SHORT:
            strncpy(vars[var_count].name, tokens[i + 1].name, sizeof(vars[var_count].name) - 1);
            vars[var_count].index = 2;
            var_count++;
            break;
        
        case TOKEN_INT:
            strncpy(vars[var_count].name, tokens[i + 1].name, sizeof(vars[var_count].name) - 1);
            vars[var_count].index = 4;
            var_count++;
            break;
        
        case TOKEN_FLOAT:
            strncpy(vars[var_count].name, tokens[i + 1].name, sizeof(vars[var_count].name) - 1);
            vars[var_count].index = 4;
            var_count++;
            break;
        
        case TOKEN_DOUBLE:
            strncpy(vars[var_count].name, tokens[i + 1].name, sizeof(vars[var_count].name) - 1);
            vars[var_count].index = 8;
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
            case TOKEN_ARROWR: 
                Token left = code.tok[i - 1];
                Token right = code.tok[i + 1];

                snprintf(code.code, sizeof(code.code), "mov ");
                break;
            default:
                break;
        }
    }
}
