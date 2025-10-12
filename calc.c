#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "calc.h"

#define MAX_TOKENS 256

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

char* eval(char *input) {
    char *tokens[MAX_TOKENS];
    int tokenCount = 0;
    for (int i = 0; input[i] != '\0'; ) {
        if (isspace((unsigned char)input[i])) { i++; continue; }

        if (isdigit((unsigned char)input[i])) {
            int start = i;
            while (isdigit((unsigned char)input[i])) i++;
            int len = i - start;
            tokens[tokenCount] = malloc(len + 1);
            strncpy(tokens[tokenCount], &input[start], len);
            tokens[tokenCount][len] = '\0';
            tokenCount++;
            continue;
        }

        if (isalpha((unsigned char)input[i])) {
            int start = i;
            while (isalnum((unsigned char)input[i])) i++;
            int len = i - start;
            tokens[tokenCount] = malloc(len + 1);
            strncpy(tokens[tokenCount], &input[start], len);
            tokens[tokenCount][len] = '\0';
            tokenCount++;
            continue;
        }

        if (strchr("+-*/", input[i])) {
            tokens[tokenCount] = malloc(2);
            tokens[tokenCount][0] = input[i];
            tokens[tokenCount][1] = '\0';
            tokenCount++;
            i++;
            continue;
        }

        i++;
    }

    char* data = malloc(1); 
    data[0] = '\0'; 
    double values[MAX_TOKENS];
    char ops[MAX_TOKENS];
    int valCount = 0, opCount = 0;

    for (int i = 0; i < tokenCount; i++) {
        if (strchr("+-*/", tokens[i][0]) && tokens[i][1] == '\0') {
            ops[opCount++] = tokens[i][0];
        } else {
            values[valCount++] = atof(tokens[i]);
        }
    }

    for (int i = 0; i < opCount; i++) {
        if (ops[i] == '*' || ops[i] == '/') {
            double result = (ops[i] == '*')
                ? 
                : values[i] / values[i + 1];
            values[i] = result;
            for (int j = i + 1; j < valCount - 1; j++)
                values[j] = values[j + 1];
            valCount--;
            for (int j = i; j < opCount - 1; j++)
                ops[j] = ops[j + 1];
            opCount--;
            i--; 
        }
    }

    double result = values[0];
    for (int i = 0; i < opCount; i++) {
        if (ops[i] == '+')
            result += values[i + 1];
        else if (ops[i] == '-')
            result -= values[i + 1];
    }

    return result;
}
