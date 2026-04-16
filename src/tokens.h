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
    TOKEN_CALL,
    TOKEN_SEC,
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

typedef struct {
    QTokenType type;
    double value;
    char name[128];
    int line;
    int column;
} Token;