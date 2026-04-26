// parser.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include "lexer.h"

int level = 0;
const char *regs[] = {
    "eax", "ecx", "edx",
    "esi", "edi",
    "r8d", "r9d", "r10d", "r11d"
};
bool reg_used[sizeof(regs)] = {false};

int alloc_reg() {
    for (int i = 0; i < 4; i++) {
        if (!reg_used[i]) {
            reg_used[i] = true;
            return i;
        }
    }
    fprintf(stderr, "Out of registers\n");
    exit(1);
}

void free_reg(int r) {
    reg_used[r] = false;
}

// Adds a string to an string
char* append(char* str, const char* add_str) {
    int len = str ? strlen(str) : 0;
    int add_len = strlen(add_str);
    char* new_str = realloc(str, len + add_len + 1);
    if (!new_str) { fprintf(stderr, "Memory allocation failed\n"); exit(1); }
    strcpy(new_str + len, add_str);
    return new_str;
}

enum Type {
    TYPE_BYTE,
    TYPE_SHORT,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_UNDEF
};

typedef struct {
    char name[32];
    int offset; // stack offset
    enum Type type; // 1: byte, 2: short, 4: int, 5: float, 8: double
} Var;

typedef struct {
    Token *tok;
    char *code;
} Parser;

int find_var_by_name(const char *name, Var *vars, int var_count) {
    for (int i = 0; i < var_count; i++)
        if (strcmp(vars[i].name, name) == 0) return i;
    return -1;
}

enum Type load(char *l, size_t l_size, Token *tok, Var *vars, int var_count) {
    if (tok->type == TOKEN_ID) {
        int var_index = find_var_by_name(tok->name, vars, var_count);
        if (var_index < 0) return TYPE_UNDEF;
        switch (vars[var_index].type) {
            case TYPE_BYTE:
                snprintf(l, l_size, "movzx eax, BYTE PTR [rbp-%d]", vars[var_index].offset);
                return TYPE_BYTE;
            case TYPE_SHORT:
                snprintf(l, l_size, "movzx eax, WORD PTR [rbp-%d]", vars[var_index].offset);
                return TYPE_SHORT;
            case TYPE_INT:
                snprintf(l, l_size, "mov eax, DWORD PTR [rbp-%d]", vars[var_index].offset);
                return TYPE_INT;
            case TYPE_FLOAT:
                snprintf(l, l_size, "movss xmm0, DWORD PTR [rbp-%d]", vars[var_index].offset);
                return TYPE_FLOAT;
            case TYPE_DOUBLE:
                snprintf(l, l_size, "movsd xmm0, QWORD PTR [rbp-%d]", vars[var_index].offset);
                return TYPE_DOUBLE;
            default: break;
        }
    } else if (tok->type == TOKEN_NUMBER) {
        l[0] = '\0';
        return TYPE_INT;
    }
    return TYPE_UNDEF;
}

static void make_indent(char *buf, size_t buf_size) {
    int spaces = level * 4;
    if ((size_t)spaces >= buf_size) spaces = (int)buf_size - 1;
    memset(buf, ' ', spaces);
    buf[spaces] = '\0';
}

enum Expr { NUM, VAR, ADD, SUB, MUL, DIV, AND, OR, XOR, NOT };
typedef struct Node {
    enum Expr type;
    struct Node *left;
    struct Node *right;
    int value;
    char name[32];
} Node;

Node* parse_expr(Token *tokens, int *pos);
Node* parse_term(Token *tokens, int *pos);
Node* parse_factor(Token *tokens, int *pos);

Node* make_node(enum Expr type, Node *left, Node *right, int value, const char *name) {
    Node *n = malloc(sizeof(Node));
    n->type = type;
    n->left = left;
    n->right = right;
    n->value = value;
    if (name) strncpy(n->name, name, 31);
    n->name[31] = '\0';
    return n;
}

Node* parse_factor(Token *tokens, int *pos) {
    Token t = tokens[*pos];

    if (t.type == TOKEN_NUMBER) {
        (*pos)++;
        return make_node(NUM, NULL, NULL, t.value, NULL);
    }

    if (t.type == TOKEN_ID) {
        (*pos)++;
        return make_node(VAR, NULL, NULL, 0, t.name);
    }

    if (t.type == TOKEN_LPAREN) {
        (*pos)++;
        Node *n = parse_expr(tokens, pos);
        (*pos)++; // )
        return n;
    }

    return NULL;
}

Node* parse_term(Token *tokens, int *pos) {
    Node *left = parse_factor(tokens, pos);

    while (tokens[*pos].type == TOKEN_MUL ||
           tokens[*pos].type == TOKEN_DIV) {

        Token op = tokens[*pos];
        (*pos)++;

        Node *right = parse_factor(tokens, pos);

        if (op.type == TOKEN_MUL) {
            left = make_node(MUL, left, right, 0, NULL);
        } else if (op.type == TOKEN_DIV) {
            left = make_node(DIV, left, right, 0, NULL);
        } else if (op.type == TOKEN_NOT) {
            left = make_node(NOT, left, right, 0, NULL);
        }
    }

    return left;
}

Node* parse_expr(Token *tokens, int *pos) {
    Node *left = parse_term(tokens, pos);

    while (tokens[*pos].type == TOKEN_MINUS || tokens[*pos].type == TOKEN_PLUS
        || tokens[*pos].type == TOKEN_AND || tokens[*pos].type == TOKEN_OR
        || tokens[*pos].type == TOKEN_XOR) {
        Token op = tokens[*pos];
        (*pos)++;

        Node *right = parse_term(tokens, pos);

        if (op.type == TOKEN_PLUS) {
            left = make_node(ADD, left, right, 0, NULL);
        } else if (op.type == TOKEN_MINUS) {
            left = make_node(SUB, left, right, 0, NULL);
        } else if (op.type == TOKEN_AND) {
            left = make_node(AND, left, right, 0, NULL);
        } else if (op.type == TOKEN_XOR) {
            left = make_node(XOR, left, right, 0, NULL);
        } else if (op.type == TOKEN_OR) {
            left = make_node(OR, left, right, 0, NULL);
        }
    }

    return left;
}

void gen(Node *n, Var *vars, int var_count, char **s) {
    char line[64];
    if (n->type == NUM) {
        snprintf(line, (size_t)64, "mov eax, %d\n", n->value);
        *s = append(*s, line);
        return;
    }

    if (n->type == VAR) {
        int idx = find_var_by_name(n->name, vars, var_count);
        if (idx < 0) {
            fprintf(stderr, "Unknown variable %s\n", n->name);
            exit(1);
        }
        snprintf(line, (size_t)64, "mov eax, DWORD PTR [rbp-%d]\n", vars[idx].offset);
        *s = append(*s, line);
        return;
    }

    // LEFT
    gen(n->left, vars, var_count, s);
    *s = append(*s, "push rax\n");

    // RIGHT
    gen(n->right, vars, var_count, s);
    *s = append(*s, "mov, ebx, eax\npop rax\n");

    // NOW:
    // eax = left
    // ebx = right
    switch (n->type) {
        case ADD:
            *s = append(*s, "add eax, ebx\n");
            break;

        case SUB:
            *s = append(*s, "sub eax, ebx\n");
            break;

        case MUL:
            *s = append(*s, "imul eax, ebx\n");
            break;
        
        case AND:
            *s = append(*s, "and eax, ebx\n");
            break;
        
        case XOR:
            *s = append(*s, "xor eax, ebx\n");
            break;
        
        case OR:
            *s = append(*s, "or eax, ebx\n");
            break;

        case DIV:
            *s = append(*s, "cdq\nidiv ebx\n");
            break;
    }
}

void printNode(Node *n, int indent) {
    for (int i = 0; i < indent; i++) printf("  ");

    switch (n->type) {
        case NUM: printf("%d\n", n->value); break;
        case VAR: printf("%s\n", n->name); break;
        case ADD: printf("+\n"); break;
        case SUB: printf("-\n"); break;
        case MUL: printf("*\n"); break;
        case DIV: printf("/\n"); break;
    }

    if (n->left) printNode(n->left, indent + 1);
    if (n->right) printNode(n->right, indent + 1);
}

// move: generates assembly for: loaded = moved
// loaded: destination (variable or register name)
// moved:  source (variable or number literal)
void move(Token *loaded, Token *moved, char *s, size_t size, Var *vars, int var_count) {
    char l[256];
    size_t l_size = sizeof(l);
    char indent[128];
    make_indent(indent, sizeof(indent));
 
    if (moved->type == TOKEN_ID) {
        // Source is a variable  load it first
        enum Type t = load(l, l_size, moved, vars, var_count);
 
        int dst_index = find_var_by_name(loaded->name, vars, var_count);
 
        if (dst_index >= 0 && loaded->type == TOKEN_ID) {
            // var = var
            switch (t) {
                case TYPE_BYTE:
                    snprintf(s, size, "%s\n%smov BYTE PTR [rbp-%d], al",
                        l, indent, vars[dst_index].offset); break;
                case TYPE_SHORT:
                    snprintf(s, size, "%s\n%smov WORD PTR [rbp-%d], ax",
                        l, indent, vars[dst_index].offset); break;
                case TYPE_INT:
                    snprintf(s, size, "%s\n%smov DWORD PTR [rbp-%d], eax",
                        l, indent, vars[dst_index].offset); break;
                case TYPE_FLOAT:
                    snprintf(s, size, "%s\n%smovss DWORD PTR [rbp-%d], xmm0",
                        l, indent, vars[dst_index].offset); break;
                case TYPE_DOUBLE:
                    snprintf(s, size, "%s\n%smovsd QWORD PTR [rbp-%d], xmm0",
                        l, indent, vars[dst_index].offset); break;
                default: break;
            }
        } else {
            // reg = var  (e.g. mov rax, QWORD PTR [rbp-8])
            int src_index = find_var_by_name(moved->name, vars, var_count);
            if (src_index < 0) return;
            switch (t) {
                case TYPE_BYTE:
                    snprintf(s, size, "%smovzx %s, BYTE PTR [rbp-%d]",
                        indent, loaded->name, vars[src_index].offset); break;
                case TYPE_SHORT:
                    snprintf(s, size, "%smovzx %s, WORD PTR [rbp-%d]",
                        indent, loaded->name, vars[src_index].offset); break;
                case TYPE_INT:
                    snprintf(s, size, "%smov %s, DWORD PTR [rbp-%d]",
                        indent, loaded->name, vars[src_index].offset); break;
                case TYPE_FLOAT:
                    snprintf(s, size, "%smovss %s, DWORD PTR [rbp-%d]",
                        indent, loaded->name, vars[src_index].offset); break;
                case TYPE_DOUBLE:
                    snprintf(s, size, "%smovsd %s, QWORD PTR [rbp-%d]",
                        indent, loaded->name, vars[src_index].offset); break;
                default: break;
            }
        }
 
    } else if (moved->type == TOKEN_NUMBER) {
        // Source is a number literal
        int dst_index = find_var_by_name(loaded->name, vars, var_count);
 
        if (dst_index >= 0 && loaded->type == TOKEN_ID) {
            // var = immediate
            enum Type t = vars[dst_index].type;
            switch (t) {
                case TYPE_BYTE:
                    snprintf(s, size, "%smov BYTE PTR [rbp-%d], %d ; def",
                        indent, vars[dst_index].offset, (int)moved->value); break;
                case TYPE_SHORT:
                    snprintf(s, size, "%smov WORD PTR [rbp-%d], %d ; def",
                        indent, vars[dst_index].offset, (int)moved->value); break;
                case TYPE_INT:
                    snprintf(s, size, "%smov DWORD PTR [rbp-%d], eax ; def",
                        indent, vars[dst_index].offset, (int)moved->value); break;
                case TYPE_FLOAT: {
                    // float immediate  eax  xmm0  memory
                    float f = (float)moved->value;
                    uint32_t bits;
                    memcpy(&bits, &f, sizeof(bits));
                    snprintf(s, size,
                        "%smov eax, 0x%08X\n%smovd xmm0, eax\n%smovss DWORD PTR [rbp-%d], xmm0 ; def",
                        indent, bits, indent, indent, vars[dst_index].offset);
                    break;
                }
                case TYPE_DOUBLE: {
                    // double immediate  rax  xmm0  memory
                    double d = moved->value;
                    uint64_t bits;
                    memcpy(&bits, &d, sizeof(bits));
                    snprintf(s, size,
                        "%smov rax, 0x%016lX\n%smovq xmm0, rax\n%smovsd QWORD PTR [rbp-%d], xmm0 ; def",
                        indent, bits, indent, indent, vars[dst_index].offset);
                    break;
                }
                default: break;
            }
        } else {
            // reg = immediate
            if (dst_index < 0) return;
            switch (vars[dst_index].type) {
                case TYPE_FLOAT: {
                    float f = (float)moved->value;
                    uint32_t bits;
                    memcpy(&bits, &f, sizeof(bits));
                    snprintf(s, size, "%smov eax, 0x%08X\n%smovd %s, eax",
                        indent, bits, indent, loaded->name);
                    break;
                }
                case TYPE_DOUBLE: {
                    double d = moved->value;
                    uint64_t bits;
                    memcpy(&bits, &d, sizeof(bits));
                    snprintf(s, size, "%smov rax, 0x%016lX\n%smovq %s, rax",
                        indent, bits, indent, loaded->name);
                    break;
                }
                default:
                    snprintf(s, size, "%smov %s, %d",
                        indent, loaded->name, (int)moved->value);
                    break;
            }
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

    Parser code;
    code.code = malloc(1); 
    code.code[0] = '\0'; 
    code.tok = tokens;
    bool default_case = false;
    Node *n;
    for (int i = 0; i < count; i++) {
        char line[512];
        switch (tokens[i].type) {
            case TOKEN_ARROWL:
                move(&tokens[i - 1], &tokens[i + 1], line, (size_t)512, vars, var_count);
                code.code = append(code.code, line);
                default_case = false;
                break;
            
            case TOKEN_ARROWR:
                default_case = false;
                if (tokens[i + 1].type == TOKEN_BYTE || tokens[i + 1].type == TOKEN_SHORT || tokens[i + 1].type == TOKEN_INT ||
                    tokens[i + 1].type == TOKEN_FLOAT || tokens[i + 1].type == TOKEN_DOUBLE) {
                    // var declaration with initialization, e.g. "10 -> int x"
                    move(&tokens[i + 2], &tokens[i - 1], line, (size_t)512, vars, var_count);

                } else {
                    move(&tokens[i + 1], &tokens[i - 1], line, (size_t)512, vars, var_count);
                }
                code.code = append(code.code, line);
                break;
            
            case TOKEN_NEWLINE:
                code.code = append(code.code, "\n");
                default_case = false;
                break;
            
            case TOKEN_NUMBER:
                n = parse_expr(tokens, &i);
                gen(n, vars, var_count, &code.code);   
                default_case = false;
                break;

            default:
                default_case = true;
                break;
        }
        if (!default_case) {
            code.code = append(code.code, "\n");
        }
    }

    return code;
}

// test
int main() {
    const char *code = 
        "int y <- 4 & 4\n";

    int count = 0;
    Token *tokens = tokenize(code, &count);
    Parser parsed = parse(tokens, count);

    printf(parsed.code);

    free(tokens);
    return 0;
}
