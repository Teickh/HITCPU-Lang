#ifndef STRUCT_HEADER_H
#define STRUCT_HEADER_H

#include <stdint.h>

#define TOKEN_LIST(X) \
    X(TOKEN_IDENTIFIER,     "IDENTIFIER")   \
    X(TOKEN_INT_LIT,        "NUMBER")       \
    X(TOKEN_CHAR_STRING,    "STRING")       \
    X(TOKEN_CHAR_LIT,       "CHARACTER")    \
    X(TOKEN_VAR_DECL,       "VARIABLE DECL")\
    X(TOKEN_INT,            "INT")          \
    X(TOKEN_CHAR,           "CHAR")         \
    X(TOKEN_VOID,           "VOID")         \
    X(TOKEN_PRINT,          "PRINT")        \
    X(TOKEN_IF,             "IF")           \
    X(TOKEN_RETURN,         "RETURN")       \
    X(TOKEN_ASSIGN,         "=")            \
    X(TOKEN_PLUS,           "+")            \
    X(TOKEN_MINUS,          "-")            \
    X(TOKEN_MULTIPLY,       "*")            \
    X(TOKEN_DIVIDE,         "/")            \
    X(TOKEN_COMPARE_EQ,     "==")           \
    X(TOKEN_NOT_EQ,         "!=")           \
    X(TOKEN_GREATER_THAN,   ">")            \
    X(TOKEN_LESS_THAN,      "<")            \
    X(TOKEN_LPAREN,         "(")            \
    X(TOKEN_RPAREN,         ")")            \
    X(TOKEN_LBRACES,        "{")            \
    X(TOKEN_RBRACES,        "}")            \
    X(TOKEN_LBRACKET,       "[")            \
    X(TOKEN_RBRACKET,       "]")            \
    X(TOKEN_COMMA,          ",")            \
    X(TOKEN_SEMICOLON,      ";")            \
    X(TOKEN_EOF,            "EOF")          \
    X(TOKEN_UNKNOWN,        "UNKNOWN")      \
    X(TOKEN_BLOCK,          "BLOCK")        \
    X(TOKEN_FUNCTION,       "FUNCTION")     \
    X(TOKEN_PARAM,          "PARAM")        \

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
    } data;
} ASTNode;

typedef struct {
    ASTNode *nodes;
    int count;
    int capacity;
} ASTTree;

typedef struct {
    int main_function_node_index;
    int *global_functions;
    int function_count;
    int function_capacity;
    int *global_variables;
    int global_var_count;
    int global_var_capacity;
} ProgramRegistry;

typedef enum {
    SYMBOL_GLOBAL,
    SYMBOL_LOCAL,
    SYMBOL_FUNCTION
} SymbolType;

typedef struct {
    int string_offset;
    SymbolType type;
    int data_type;
    int stack_slot;
} Symbol;

typedef struct Scope {
    Symbol *symbols;
    int symbol_count;
    int symbol_capacity;
    int parent_scope_idx;
} Scope;

typedef struct {
    Scope *scopes;
    int scope_count;
    int scope_capacity;
    int root_scope_idx;
} ScopeTree;

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
    
    OP_MAX_COUNT // Useful helper to keep track of array limits
} OpcodeType;

typedef enum {
    REG_R0 = 0, REG_R1, REG_R2, REG_R3, REG_R4,
    REG_R5, REG_R6, REG_R7, REG_R8, REG_R9, REG_R10,
    REG_R11, REG_R12, REG_R13, REG_R14, REG_R15, // REG_R15 is the sp reg. R0 is the zero reg.
} Register;

// Represents an unrolled instruction in your stream
typedef struct {
    OpcodeType op;

    // Abstract operands that any backend can interpret or lower
    Register dest;
    Register src1;
    union {
        Register src2;
        int32_t imm;
        int32_t offset; // For memory offsets or branch targets
    } src2_or_imm;

    // Control flags for optimization passes
    int is_dead; // Mark true if an optimization pass decides to delete this
} AsmInstruction;

typedef struct {
    AsmInstruction* data;
    int capacity;
    int count;
} InstructionStream;

#endif