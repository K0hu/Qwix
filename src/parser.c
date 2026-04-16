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
    Token **tok;
    char code[1024];
} Parser;

char load(Token *tok, int offset) {
    
}

char save(Token *tok, int offset) {

}

Parser parse(Token *tokens, int count) {
    int level = 0;
    Parser code;
    code.tok = *tokens
    for (int i = 0; i < count; i++) {
        switch (code.tok[i].type) {
            case TOKEN_ARROWR: 
                Token left = code.tok[i - 1];
                Token right = code.tok[i + 1];

                snprintf(code.code, size_of(code.code), "mov ");
        }
    }
}
