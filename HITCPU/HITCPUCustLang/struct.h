#ifndef STRUCT_HEADER_H
#define STRUCT_HEADER_H

#include <stdint.h>

#define TOKEN_LIST(X) \
    X(TOKEN_IDENTIFIER,         "IDENTIFIER")   \
    X(TOKEN_INT_LIT,            "NUMBER")       \
    X(TOKEN_CHAR_STRING,        "STRING")       \
    X(TOKEN_CHAR_LIT,           "CHARACTER")    \
    X(TOKEN_VAR_DECL,           "VARIABLE DECL")\
    X(TOKEN_INT,                "INT")          \
    X(TOKEN_CHAR,               "CHAR")         \
    X(TOKEN_VOID,               "VOID")         \
    X(TOKEN_IF,                 "IF")           \
    X(TOKEN_ELSE,               "ELSE")         \
    X(TOKEN_RETURN,             "RETURN")       \
    X(TOKEN_ASSIGN,             "=")            \
    X(TOKEN_PLUS,               "+")            \
    X(TOKEN_MINUS,              "-")            \
    X(TOKEN_MULTIPLY,           "*")            \
    X(TOKEN_DIVIDE,             "/")            \
    X(TOKEN_COMPARE_EQ,         "==")           \
    X(TOKEN_NOT_EQ,             "!=")           \
    X(TOKEN_GREATER_THAN,       ">")            \
    X(TOKEN_GREATER_THAN_OR_EQ, ">=")           \
    X(TOKEN_LESS_THAN,          "<")            \
    X(TOKEN_LESS_THAN_OR_EQ,    "<=")           \
    X(TOKEN_AND,                "&&")           \
    X(TOKEN_OR,                 "||")           \
    X(TOKEN_LPAREN,             "(")            \
    X(TOKEN_RPAREN,             ")")            \
    X(TOKEN_LBRACES,            "{")            \
    X(TOKEN_RBRACES,            "}")            \
    X(TOKEN_LBRACKET,           "[")            \
    X(TOKEN_RBRACKET,           "]")            \
    X(TOKEN_COMMA,              ",")            \
    X(TOKEN_SEMICOLON,          ";")            \
    X(TOKEN_EOF,                "EOF")          \
    X(TOKEN_UNKNOWN,            "UNKNOWN")      \
    X(TOKEN_BLOCK,              "BLOCK")        \
    X(TOKEN_FUNCTION,           "FUNCTION")     \
    X(TOKEN_PARAM,              "PARAM")        \
    X(TOKEN_ARG_LIST,           "ARGUMENTS")    \
    X(TOKEN_FUNCTION_CALL,      "FUNCTION CALL")\
    X(TOKEN_CMP_LIST,           "COMPARES")     \

typedef enum {
    #define AS_ENUM(ENUM, STR) ENUM,
    TOKEN_LIST(AS_ENUM)
    #undef AS_ENUM
} TokenType;

static inline const char* token_type_to_string(TokenType type) {
    switch (type) {
        #define AS_STRING(ENUM, STR) case ENUM: return STR;
        TOKEN_LIST(AS_STRING)
        #undef AS_STRING
        default: return "UNKNOWN";
    }
}

typedef struct {
    TokenType type;
    int value_offset;
    int line;
    int column;
} Token;

typedef struct {
    Token *items;
    int count;
    int capacity;
} TokenList;

typedef struct {
    char *data;
    int size;
    int capacity;
} StringPool;

typedef struct {
    int *statement_indices;
    int statement_count;
} BlockData;

typedef struct {
    int *param_indices;
    int param_count;
} ParamData;

typedef struct {
    TokenType return_type;
    int name_string_offset;
} FunctionData;

typedef struct {
    int condition_idx;
    int true_block_idx;
    int false_block_idx;
} IfStatement;

typedef struct ASTNode {
    TokenType type;
    int left;
    int right;
    int line;
    int column;
    union {
        int number_value;
        int string_offset;
        TokenType variable_type;
        BlockData block;
        FunctionData function;
        ParamData param;
        IfStatement if_statement;
    } data;
} ASTNode;

typedef struct {
    ASTNode *nodes;
    int count;
    int capacity;
} ASTTree;

typedef struct {
    const char *filename;
    int main_function_node_index;
    int *global_functions;
    int function_count;
    int function_capacity;
    int *global_variables;
    int global_var_count;
    int global_var_capacity;
} FileRegistry;

typedef struct {
    int main_file_index;
    int main_function_index;
    FileRegistry *files;
    int count;
    int capacity;
} ProgramList;

typedef enum {
    SYMBOL_GLOBAL,
    SYMBOL_LOCAL,
    SYMBOL_FUNCTION
} SymbolType;

typedef struct {
    int string_offset;
    SymbolType type;
    TokenType data_type;
} Symbol;

typedef struct {
    Symbol *symbols;
    int symbol_count;
    int symbol_capacity;
    int parent_scope;
} SymbolTable;

typedef struct {
    SymbolTable *scopes;
    int scope_capacity;
    int stack;
} ScopeStack;

typedef struct {
    SymbolTable *historical_scopes;
    int count;
    int capacity;
} SymbolLists;

#define VREG_R0  1000
#define VREG_R14 1014
#define VREG_R15 1015

typedef enum {
    OP_NONE = 0,

    // ==================== R-type ====================
    OP_ADD,
    OP_SUB,
    OP_MOV,
    OP_AND,
    OP_OR,
    OP_XOR,
    OP_INC,
    OP_DEC,
    OP_LSL,
    OP_LSR,
    OP_ROL,
    OP_ROR,
    OP_ASR,
    OP_NAND,
    OP_NOR,
    OP_XNOR,
    OP_NOT,
    OP_NEG,
    OP_RSB,
    OP_ADC,
    OP_SBC,
    OP_MUL,
    OP_DIV,

    // ==================== I-type ====================
    OP_ADDI,
    OP_SUBI,
    OP_MOVI,
    OP_ANDI,
    OP_ORI,
    OP_XORI,
    OP_LSLI,
    OP_LSRI,
    OP_ROLI,
    OP_RORI,
    OP_ASRI,
    OP_NANDI,
    OP_NORI,
    OP_XNORI,
    OP_RSBI,
    OP_ADCI,
    OP_SBCI,

    // ==================== F-type ====================
    OP_ADDF,
    OP_SUBF,
    OP_ANDF,
    OP_ORF,
    OP_XORF,
    OP_INCF,
    OP_DECF,
    OP_LSLF,
    OP_LSRF,
    OP_ROLF,
    OP_RORF,
    OP_ASRF,
    OP_NANDF,
    OP_NORF,
    OP_XNORF,
    OP_RSBF,
    OP_ADCF,
    OP_SBCF,

    OP_CMP,
    OP_TST,
    OP_TEQ,
    OP_CMPI,
    OP_TSTI,
    OP_TEQI,

    // ==================== Special types ====================
    OP_CLV,
    OP_CLC,
    OP_CLZ,
    OP_CLN,
    OP_SEV,
    OP_SEC,
    OP_SEZ,
    OP_SEN,

    OP_BTST,

    // ==================== B-type ====================
    OP_JMP,
    OP_BEQ,
    OP_BNE,
    OP_BGT,
    OP_BLT,
    OP_BGE,
    OP_BLE,
    OP_BCS,
    OP_BCC,
    OP_BMI,
    OP_BPL,
    OP_CALL,
    OP_RET,

    OP_NOP,
    OP_HLT,

    // =================== Mem-Type ===================
    OP_LDR,
    OP_STR,
    
    OP_LABEL,

    OP_MAX_COUNT // Useful helper to keep track of array limits
} OpcodeType;

typedef struct {
    OpcodeType op;
    int dest;
    int src1;
    union {
        int src2;
        int imm;
        int offset;
    } src2_or_imm;

    int is_dead;
} AsmInstruction;

typedef struct {
    AsmInstruction* data;
    int capacity;
    int count;
} InstructionStream;

typedef struct {
    int is_busy;
    int vreg_id;
    int least_used;
} Register;

typedef struct {
    Register reg[13]; // R1 - R14 register
} RegStatus;

#endif