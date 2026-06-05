#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define MAKE_OP(op, mode) (((op << 1) | (mode)))

#define MAX_LABELS 1028

typedef enum {
    FMT_NONE, // special
    FMT_R,    // [opcode][dest][reg A][reg B]
    FMT_I,    // [opcode][dest][reg A][imm]
    FMT_R2,   // [opcode][dest][reg A]
    FMT_I2,   // [opcode][dest][imm]
    FMT_FR,   // [opcode][reg A][reg B]
    FMT_FI,   // [opcode][reg A][imm]
    FMT_B,    // [opcode]TBD
    FMT_M     // [opcode]TBD
} InstFormat;

typedef struct {
    char *name;
    uint8_t opcode;
    InstFormat format;
} Instruction;

typedef struct {
    char name[32];
    int address;
} Label;

Instruction table[] = { // [5 opcode][1 wr reg][1 wr mem][1 imm]
    // R-type (wr_reg = 1, wr_mem = 0, imm = 0)
    {"ADD",   MAKE_OP(0b00000, 0b100), FMT_R},
    {"SUB",   MAKE_OP(0b00001, 0b100), FMT_R},
    {"AND",   MAKE_OP(0b00010, 0b100), FMT_R},
    {"OR",    MAKE_OP(0b00011, 0b100), FMT_R},
    {"XOR",   MAKE_OP(0b00100, 0b100), FMT_R},
    {"LSL",   MAKE_OP(0b00101, 0b100), FMT_R},
    {"LSR",   MAKE_OP(0b00110, 0b100), FMT_R},
    {"ROL",   MAKE_OP(0b00111, 0b100), FMT_R},
    {"ROR",   MAKE_OP(0b01000, 0b100), FMT_R},
    {"ASR",   MAKE_OP(0b01001, 0b100), FMT_R},
    {"NAND",  MAKE_OP(0b01010, 0b100), FMT_R},
    {"NOR",   MAKE_OP(0b01011, 0b100), FMT_R},
    {"XNOR",  MAKE_OP(0b01100, 0b100), FMT_R},
    {"NOT",   MAKE_OP(0b01101, 0b100), FMT_R},
    {"MOV",   MAKE_OP(0b01110, 0b100), FMT_R2},
    {"NEG",   MAKE_OP(0b01111, 0b100), FMT_R},

    // I-type (wr_reg = 1, wr_mem = 0, imm = 1)
    {"ADDI",  MAKE_OP(0b00000, 0b101), FMT_I},
    {"SUBI",  MAKE_OP(0b00001, 0b101), FMT_I},
    {"ANDI",  MAKE_OP(0b00010, 0b101), FMT_I},
    {"ORI",   MAKE_OP(0b00011, 0b101), FMT_I},
    {"XORI",  MAKE_OP(0b00100, 0b101), FMT_I},
    {"LSLI",  MAKE_OP(0b00101, 0b101), FMT_I},
    {"LSRI",  MAKE_OP(0b00110, 0b101), FMT_I},
    {"ROLI",  MAKE_OP(0b00111, 0b101), FMT_I},
    {"RORI",  MAKE_OP(0b01000, 0b101), FMT_I},
    {"ASRI",  MAKE_OP(0b01001, 0b101), FMT_I},
    {"NANDI", MAKE_OP(0b01010, 0b101), FMT_I},
    {"NORI",  MAKE_OP(0b01011, 0b101), FMT_I},
    {"XNORI", MAKE_OP(0b01100, 0b101), FMT_I},
    {"MOVI",  MAKE_OP(0b01110, 0b101), FMT_I2},

    // B-type (wr_reg = 0, wr_mem = 0, imm = 1)
    {"LOAD",  MAKE_OP(0b00000, 0b101), FMT_M},
    {"STR",   MAKE_OP(0b00001, 0b011), FMT_M},

    // B-type (wr_reg = 0, wr_mem = 0, imm = 1)
    {"BEQ",   MAKE_OP(0b00001, 0b001), FMT_B},
    {"BNE",   MAKE_OP(0b00010, 0b001), FMT_B},
    {"BLTU",  MAKE_OP(0b00011, 0b001), FMT_B},
    {"BGTU",  MAKE_OP(0b00100, 0b001), FMT_B},
    {"BLTS",  MAKE_OP(0b00101, 0b001), FMT_B},
    {"BGTS",  MAKE_OP(0b00110, 0b001), FMT_B},
    {"BLEU",  MAKE_OP(0b00111, 0b001), FMT_B},
    {"BGEU",  MAKE_OP(0b01000, 0b001), FMT_B},
    {"BLES",  MAKE_OP(0b01001, 0b001), FMT_B},
    {"BGES",  MAKE_OP(0b01010, 0b001), FMT_B},

    {"JMP",   MAKE_OP(0b01011, 0b001), FMT_B},
    {"CALL",  MAKE_OP(0b01100, 0b101), FMT_B}, // wr_reg = 1 to save return address
    {"RET",   MAKE_OP(0b01101, 0b001), FMT_B},

    {"NOP",   MAKE_OP(0b00000, 0b000), FMT_NONE},
    {"HLT",   MAKE_OP(0b11111, 0b000), FMT_NONE}
};

const size_t num_of_instruction = sizeof(table) / sizeof(table[0]);

FILE *open_file(const char *program_name) {
    char file_path[256];

    snprintf(file_path, sizeof(file_path), "programs/%s.as", program_name);

    FILE *program = fopen(file_path, "r");

    if (program == NULL) {
        fprintf(stderr, "Attempted to open: %s\n", file_path);
        perror("Error opening file");
        return NULL;
    }
    
    printf("File %s open successfully!", file_path);
    return program;
}

FILE *create_bin_file(const char *program_name) {
    char file_path[256];

    snprintf(file_path, sizeof(file_path), "programs/%s.bin", program_name);

    FILE *program_bin = fopen(file_path, "wb");

    fprintf(stderr, "\nCreating file...");

    if (program_bin == NULL) {
        fprintf(stderr, "Attempted to create: %s\n", file_path);
        perror("Error creating file");
        return NULL;
    }
    
    printf("File %s created successfully!\n", file_path);
    return program_bin;
}

void check_register_bounds(int dest, int regA, int regB) {
    if (dest < 0 || dest > 15 || regA < 0 || regA > 15 || regB < 0 || regB > 15) {
        fprintf(stderr, "ERROR: Unknown register space. Register is only R1 to R15.\n");
        exit(1);
    }
}

void resolve_register(char *arg) {
    if (arg[0] != 'r') {
        fprintf(stderr, "ERROR: Unknown instruction '%s' on this line.\n", arg);
        exit(1);
    }
}

void assembling(FILE *program, FILE *program_bin) {
    Label labels[MAX_LABELS]; // 1028 now cuz my RAM is 2^10 big lmao
    for (int i = 0; i < MAX_LABELS; i++)
        labels[i].address = -1;

    int label_count = 0;
    char line[256];
    int addr = 0;

    while (fgets(line, sizeof(line), program)) {
        int found_label = 0;
        char *find_label = strtok(line, " \t\n,");

        if (find_label == NULL || strcmp(find_label, ";") == 0) continue;
        
        if (strlen(find_label) > 0 && find_label[strlen(find_label) - 1] != ':'){
            addr++;
            continue;
        }

        find_label[strlen(find_label) - 1] = '\0';

        size_t i;
        for (i = 0; i < MAX_LABELS; i++) {
            if (labels[i].address != -1)
                continue;

            strcpy(labels[label_count].name, find_label);
            labels[i].address = addr;
            label_count++;
            found_label++;
            break;
        }
    }
    
    rewind(program);
    addr = 0;

    while(fgets(line, sizeof(line), program)) {
        char *comment = strchr(line, ';');
        if (comment) *comment = '\0';

        uint32_t instruction = 0;

        char *opcode = strtok(line, " \t\n,");
        
        if (opcode == NULL || strcmp(opcode, ";") == 0 || opcode[strlen(opcode) - 1] == ':') continue;

        size_t i;
        for (i = 0; i < num_of_instruction; i++) {
            if (strcmp(opcode, table[i].name) == 0) {
                instruction = (table[i].opcode);
                break;
            }
        }

        if (i == num_of_instruction) {
            fprintf(stderr, "ERROR: Unknown instruction '%s' on this line.\n", opcode);
            exit(1);
        }
        
        int dest = 0, regA = 0, regB = 0, imm = 0;

        char *arg1 = strtok(NULL, " \t\n,");
        char *arg2 = strtok(NULL, " \t\n,");
        char *arg3 = strtok(NULL, " \t\n,");
        char *arg4 = strtok(NULL, "\n\0");

        if (strcmp(opcode, "INC") == 0 || strcmp(opcode, "DEC") == 0 || strcmp(opcode, "INCF") == 0 || strcmp(opcode, "DECF") == 0) {
            arg2 = arg1;
        }

        switch (table[i].format) {
            case FMT_NONE: // special
                if (arg1 != NULL || arg2 != NULL || arg3 != NULL || arg4 != NULL) {
                    fprintf(stderr, "ERROR: Unknown instruction '%s' on this line.\n", arg4);
                    exit(1);
                }
                break;

            case FMT_R: // [opcode][dest][reg A][reg B]
                if (arg4 != NULL) {
                    fprintf(stderr, "ERROR: Unknown instruction '%s' on this line.\n", arg4);
                    exit(1);
                }
            
                if (arg1) resolve_register(arg1);
                if (arg2) resolve_register(arg2);
                if (arg3) resolve_register(arg3);
                
                dest = (arg1) ? atoi(arg1 + 1) : 0;
                regA = (arg2) ? atoi(arg2 + 1) : 0;
                regB = (arg3) ? atoi(arg3 + 1) : 0;
                instruction |= (dest << 8) | (regA << 12) | (regB << 16);
                break;

            case FMT_I: // [opcode][dest][reg A][imm]
                if (arg4 != NULL) {
                    fprintf(stderr, "ERROR: Unknown instruction '%s' on this line.\n", arg4);
                    exit(1);
                }

                if (arg1) resolve_register(arg1);
                if (arg2) resolve_register(arg2);
                
                dest = (arg1) ? atoi(arg1 + 1) : 0;
                regA = (arg2) ? atoi(arg2 + 1) : 0;
                imm = (arg3) ? atoi(arg3) : 0;
                instruction |= (dest << 8) | (regA << 12) | (imm & 0xFFFF) << 16;
                break;

            case FMT_R2: // [opcode][dest][reg A][0]
                if (arg3 != NULL || arg4 != NULL) {
                    fprintf(stderr, "ERROR: Unknown instruction '%s' on this line.\n", arg4);
                    exit(1);
                }
                
                if (arg1) resolve_register(arg1);
                if (arg2) resolve_register(arg2);
                
                dest = (arg1) ? atoi(arg1 + 1) : 0;
                regA = (arg2) ? atoi(arg2 + 1) : 0;
                instruction |= (dest << 8) | (regA << 12);
                break;

            case FMT_I2: // [opcode][dest][0][imm]
                if (arg3 != NULL || arg4 != NULL) {
                    fprintf(stderr, "ERROR: Unknown instruction '%s' on this line.\n", arg4);
                    exit(1);
                }
                
                if (arg1) resolve_register(arg1);
                
                dest = (arg1) ? atoi(arg1 + 1) : 0;
                imm = (arg2) ? atoi(arg2) : 0;
                instruction |= (dest << 8) | (imm & 0xFFFF) << 16;
                break;

            case FMT_FR: // [opcode][0][reg A][reg b]
                if (arg3 != NULL || arg4 != NULL) {
                    fprintf(stderr, "ERROR: Unknown instruction '%s' on this line.\n", arg4);
                    exit(1);
                }
                
                if (arg1) resolve_register(arg1);
                if (arg2) resolve_register(arg2);
                
                regA = (arg1) ? atoi(arg1 + 1) : 0;
                regB = (arg2) ? atoi(arg2 + 1) : 0;
                instruction |= (regA << 12) | (regB << 16);
                break;

            case FMT_FI: // [opcode][0][reg A][imm]
                if (arg3 != NULL || arg4 != NULL) {
                    fprintf(stderr, "ERROR: Unknown instruction '%s' on this line.\n", arg4);
                    exit(1);
                }
                
                if (arg1) resolve_register(arg1);
                
                regA = (arg1) ? atoi(arg1 + 1) : 0;
                imm = (arg2) ? atoi(arg2) : 0;
                instruction |= (regA << 12) | (imm & 0xFFFF) << 16;
                break;

            case FMT_B: // [opcode][0][0][imm]
                if (arg2 != NULL || arg3 != NULL || arg4 != NULL) {
                    fprintf(stderr, "ERROR: Unknown instruction '%s' on this line.\n", arg4);
                    exit(1);
                }

                if (arg1 == NULL) {
                    fprintf(stderr, "ERROR: Unknown instruction '%s' on this line.\n", arg1);
                    exit(1);
                }
                
                int found = 0;
                if (isdigit(arg1[0]) || arg1[0] == '-') {
                    imm = atoi(arg1);
                    break;
                } else {
                    for (size_t k = 0; k < label_count; k++) {
                        if (strcmp(arg1, labels[k].name) == 0) {
                            imm = labels[k].address - (addr + 1);
                            found = 1;
                        }
                    }
                }
                if (!found) {
                    fprintf(stderr, "ERROR: Unknown instruction '%s' on this line.\n", arg1);
                    exit(1);
                }
                
                instruction |= (imm & 0xFFFF) << 16;
                break;
            
            default:
                // I'lll just leave it here. Not sure what to do for default
                // Don't think need to do anything though
                break;
        }
        
        check_register_bounds(dest, regA, regB);

        fwrite(&instruction, sizeof(instruction), 1, program_bin);

        printf("%08x\n", instruction); // debugging

        addr++;
    }
}

int main() {
    const char *program_name = "TEST";

    FILE *program = open_file(program_name);

    if (program == NULL) {
        return 1;
    }

    FILE *program_bin = create_bin_file(program_name);
    assembling(program, program_bin);

    fclose(program);
    fclose(program_bin);
    return 0;
}
