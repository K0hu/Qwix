#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

typedef enum {
    TOKEN_INT, TOKEN_DEZ, TOKEN_ID, TOKEN_TYPE,
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_MUL, TOKEN_DIV,
    TOKEN_ARROW, TOKEN_AND, TOKEN_OR, TOKEN_XOR, TOKEN_NOT,
    TOKEN_GT, TOKEN_LT, TOKEN_LPAREN, TOKEN_RPAREN,
    TOKEN_LBRACE, TOKEN_RBRACE, TOKEN_INC, TOKEN_DEC,
    TOKEN_QUESTION, TOKEN_COLON, TOKEN_RET, TOKEN_EOF, TOKEN_SEMICOLON
} TokenType;

typedef struct {
    TokenType type;
    char value[50];
} Token;

typedef struct {
    Token tokens[1000];
    int pos;
    int count;
} Lexer;

typedef struct {
    char name[50];
    int offset;
    char type[20];
    int size;
} Symbol;

typedef struct {
    Symbol symbols[100];
    int count;
    int next_offset;
} SymbolTable;

SymbolTable symtable = {0, 0, -8};
int label_counter = 0;

void lex(const char *input, Lexer *lexer) {
    int i = 0;
    lexer->count = 0;
    
    while (input[i]) {
        if (isspace(input[i])) {
            i++;
            continue;
        }
        
        if (isdigit(input[i])) {
            int j = 0;
            while (isdigit(input[i])) {
                lexer->tokens[lexer->count].value[j++] = input[i++];
            }
            lexer->tokens[lexer->count].value[j] = '\0';
            lexer->tokens[lexer->count].type = TOKEN_INT;
            lexer->count++;
            continue;
        }
        
        if (isalpha(input[i]) || input[i] == '_') {
            int j = 0;
            while (isalnum(input[i]) || input[i] == '_') {
                lexer->tokens[lexer->count].value[j++] = input[i++];
            }
            lexer->tokens[lexer->count].value[j] = '\0';
            
            if (strcmp(lexer->tokens[lexer->count].value, "int") == 0 ||
                strcmp(lexer->tokens[lexer->count].value, "float") == 0) {
                lexer->tokens[lexer->count].type = TOKEN_TYPE;
            } else if (strcmp(lexer->tokens[lexer->count].value, "ret") == 0) {
                lexer->tokens[lexer->count].type = TOKEN_RET;
            } else {
                lexer->tokens[lexer->count].type = TOKEN_ID;
            }
            lexer->count++;
            continue;
        }
        
        if (input[i] == '+' && input[i+1] == '+') {
            lexer->tokens[lexer->count].type = TOKEN_INC;
            strcpy(lexer->tokens[lexer->count].value, "++");
            lexer->count++;
            i += 2;
        } else if (input[i] == '-' && input[i+1] == '-') {
            lexer->tokens[lexer->count].type = TOKEN_DEC;
            strcpy(lexer->tokens[lexer->count].value, "--");
            lexer->count++;
            i += 2;
        } else if (input[i] == '-' && input[i+1] == '>') {
            lexer->tokens[lexer->count].type = TOKEN_ARROW;
            strcpy(lexer->tokens[lexer->count].value, "->");
            lexer->count++;
            i += 2;
        } else if (input[i] == '+') {
            lexer->tokens[lexer->count].type = TOKEN_PLUS;
            strcpy(lexer->tokens[lexer->count].value, "+");
            lexer->count++;
            i++;
        } else if (input[i] == '-') {
            lexer->tokens[lexer->count].type = TOKEN_MINUS;
            strcpy(lexer->tokens[lexer->count].value, "-");
            lexer->count++;
            i++;
        } else if (input[i] == '*') {
            lexer->tokens[lexer->count].type = TOKEN_MUL;
            strcpy(lexer->tokens[lexer->count].value, "*");
            lexer->count++;
            i++;
        } else if (input[i] == '/') {
            lexer->tokens[lexer->count].type = TOKEN_DIV;
            strcpy(lexer->tokens[lexer->count].value, "/");
            lexer->count++;
            i++;
        } else if (input[i] == '&') {
            lexer->tokens[lexer->count].type = TOKEN_AND;
            strcpy(lexer->tokens[lexer->count].value, "&");
            lexer->count++;
            i++;
        } else if (input[i] == ':') {
            lexer->tokens[lexer->count].type = TOKEN_OR;
            strcpy(lexer->tokens[lexer->count].value, ":");
            lexer->count++;
            i++;
        } else if (input[i] == '^') {
            lexer->tokens[lexer->count].type = TOKEN_XOR;
            strcpy(lexer->tokens[lexer->count].value, "^");
            lexer->count++;
            i++;
        } else if (input[i] == '!') {
            lexer->tokens[lexer->count].type = TOKEN_NOT;
            strcpy(lexer->tokens[lexer->count].value, "!");
            lexer->count++;
            i++;
        } else if (input[i] == '>') {
            lexer->tokens[lexer->count].type = TOKEN_GT;
            strcpy(lexer->tokens[lexer->count].value, ">");
            lexer->count++;
            i++;
        } else if (input[i] == '<') {
            lexer->tokens[lexer->count].type = TOKEN_LT;
            strcpy(lexer->tokens[lexer->count].value, "<");
            lexer->count++;
            i++;
        } else if (input[i] == '?') {
            lexer->tokens[lexer->count].type = TOKEN_QUESTION;
            strcpy(lexer->tokens[lexer->count].value, "?");
            lexer->count++;
            i++;
        } else if (input[i] == '(') {
            lexer->tokens[lexer->count].type = TOKEN_LPAREN;
            strcpy(lexer->tokens[lexer->count].value, "(");
            lexer->count++;
            i++;
        } else if (input[i] == ')') {
            lexer->tokens[lexer->count].type = TOKEN_RPAREN;
            strcpy(lexer->tokens[lexer->count].value, ")");
            lexer->count++;
            i++;
        } else if (input[i] == '{') {
            lexer->tokens[lexer->count].type = TOKEN_LBRACE;
            strcpy(lexer->tokens[lexer->count].value, "{");
            lexer->count++;
            i++;
        } else if (input[i] == '}') {
            lexer->tokens[lexer->count].type = TOKEN_RBRACE;
            strcpy(lexer->tokens[lexer->count].value, "}");
            lexer->count++;
            i++;
        } else if (input[i] == ';') {
            lexer->tokens[lexer->count].type = TOKEN_SEMICOLON;
            strcpy(lexer->tokens[lexer->count].value, ";");
            lexer->count++;
            i++;
        } else {
            i++;
        }
    }
    
    lexer->tokens[lexer->count].type = TOKEN_EOF;
}

void add_symbol(const char *name, const char *type) {
    strcpy(symtable.symbols[symtable.count].name, name);
    strcpy(symtable.symbols[symtable.count].type, type);
    symtable.symbols[symtable.count].size = (strcmp(type, "float") == 0) ? 8 : 4;
    symtable.symbols[symtable.count].offset = symtable.next_offset;
    symtable.next_offset -= symtable.symbols[symtable.count].size;
    symtable.count++;
    
    printf("; Declare %s %s at [rbp%d]\n", type, name, 
           symtable.symbols[symtable.count-1].offset);
}

Symbol* find_symbol(const char *name) {
    for (int i = 0; i < symtable.count; i++) {
        if (strcmp(symtable.symbols[i].name, name) == 0) {
            return &symtable.symbols[i];
        }
    }
    return NULL;
}

int get_label() {
    return label_counter++;
}

// Parse expression und berechne Ergebnis in rax
// Gibt offset der neuen Variable zurück (oder -1 falls nicht deklariert)
int parse_expression(Lexer *lexer, int *pos) {
    // Einfache Expression: number op number
    // z.B. 1 + 1
    
    if (lexer->tokens[*pos].type == TOKEN_INT) {
        int val1 = atoi(lexer->tokens[*pos].value);
        (*pos)++;
        
        if (lexer->tokens[*pos].type == TOKEN_PLUS) {
            (*pos)++;
            int val2 = atoi(lexer->tokens[*pos].value);
            (*pos)++;
            
            int result = val1 + val2;
            printf("mov rax, %d             ; 1 + 1 = %d\n", result, result);
            return 0;  // Ergebnis ist in rax
        }
        else if (lexer->tokens[*pos].type == TOKEN_MINUS) {
            (*pos)++;
            int val2 = atoi(lexer->tokens[*pos].value);
            (*pos)++;
            
            int result = val1 - val2;
            printf("mov rax, %d             ; %d - %d = %d\n", result, val1, val2, result);
            return 0;
        }
        else if (lexer->tokens[*pos].type == TOKEN_MUL) {
            (*pos)++;
            int val2 = atoi(lexer->tokens[*pos].value);
            (*pos)++;
            
            int result = val1 * val2;
            printf("mov rax, %d             ; %d * %d = %d\n", result, val1, val2, result);
            return 0;
        }
        else if (lexer->tokens[*pos].type == TOKEN_DIV) {
            (*pos)++;
            int val2 = atoi(lexer->tokens[*pos].value);
            (*pos)++;
            
            int result = val1 / val2;
            printf("mov rax, %d             ; %d / %d = %d\n", result, val1, val2, result);
            return 0;
        }
    }
    
    return -1;  // Error
}

void compile(Lexer *lexer) {
    int pos = 0;
    
    while (lexer->tokens[pos].type != TOKEN_EOF) {
        Token t = lexer->tokens[pos];
        
        // Expression -> Type Name
        // z.B. 1 + 1 -> float b
        if ((t.type == TOKEN_INT || t.type == TOKEN_ID) && 
            lexer->tokens[pos+1].type == TOKEN_PLUS ||
            lexer->tokens[pos+1].type == TOKEN_MINUS ||
            lexer->tokens[pos+1].type == TOKEN_MUL ||
            lexer->tokens[pos+1].type == TOKEN_DIV) {
            
            // Parse expression
            parse_expression(lexer, &pos);
            
            // Expect ->
            if (lexer->tokens[pos].type == TOKEN_ARROW) {
                pos++;
                
                // Expect type
                if (lexer->tokens[pos].type == TOKEN_TYPE) {
                    const char *type = lexer->tokens[pos].value;
                    pos++;
                    
                    // Expect variable name
                    if (lexer->tokens[pos].type == TOKEN_ID) {
                        const char *name = lexer->tokens[pos].value;
                        pos++;
                        
                        // Deklariere Variable
                        if (find_symbol(name) == NULL) {
                            add_symbol(name, type);
                        }
                        
                        Symbol *sym = find_symbol(name);
                        printf("mov [rbp%d], rax        ; store result to %s\n\n", 
                               sym->offset, name);
                    }
                }
            }
        }
        // Type declaration: int/float a = ...
        else if (t.type == TOKEN_TYPE) {
            const char *type = t.value;
            pos++;
            
            if (lexer->tokens[pos].type == TOKEN_ID) {
                const char *name = lexer->tokens[pos].value;
                pos++;
                
                if (find_symbol(name) == NULL) {
                    add_symbol(name, type);
                }
                
                // Skip rest of line
                while (lexer->tokens[pos].type != TOKEN_SEMICOLON && 
                       lexer->tokens[pos].type != TOKEN_EOF) {
                    pos++;
                }
                if (lexer->tokens[pos].type == TOKEN_SEMICOLON) pos++;
            }
        }
        // Assignment: a -> b
        else if (t.type == TOKEN_ID && lexer->tokens[pos+1].type == TOKEN_ARROW) {
            const char *src = t.value;
            pos += 2;
            
            if (lexer->tokens[pos].type == TOKEN_ID) {
                const char *dst = lexer->tokens[pos].value;
                pos++;
                
                Symbol *src_sym = find_symbol(src);
                Symbol *dst_sym = find_symbol(dst);
                
                if (src_sym && dst_sym) {
                    printf("mov rax, [rbp%d]        ; load %s\n", src_sym->offset, src);
                    printf("mov [rbp%d], rax        ; store to %s\n\n", dst_sym->offset, dst);
                }
            }
        }
        // Increment: a++
        else if (t.type == TOKEN_ID && lexer->tokens[pos+1].type == TOKEN_INC) {
            Symbol *sym = find_symbol(t.value);
            if (sym) {
                printf("inc dword [rbp%d]       ; %s++\n\n", sym->offset, t.value);
            }
            pos += 2;
        }
        // Decrement: a--
        else if (t.type == TOKEN_ID && lexer->tokens[pos+1].type == TOKEN_DEC) {
            Symbol *sym = find_symbol(t.value);
            if (sym) {
                printf("dec dword [rbp%d]       ; %s--\n\n", sym->offset, t.value);
            }
            pos += 2;
        }
        // Return
        else if (t.type == TOKEN_RET) {
            printf("ret\n");
            pos++;
        }
        else {
            pos++;
        }
    }
}

int main() {
    Lexer lexer;
    
    const char *code = 
        "1 + 2 -> float c"
        "1 + 1 -> float b\n"
        "5 - 2 -> int a\n"
        "a -> b\n";
    
    lex(code, &lexer);
    
    printf("; === Generated Assembly ===\n");
    printf("section .text\n");
    printf("main:\n");
    printf("push rbp\n");
    printf("mov rbp, rsp\n");
    printf("sub rsp, 32             ; Alloc stack space\n\n");
    
    compile(&lexer);
    
    printf("\npop rbp\n");
    printf("ret\n");
    
    return 0;
}