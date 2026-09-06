#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define MAKE_OP(op, mode) (((mode << 5) | (op)))

#define MAX_LABELS 1028

typedef enum {
    FMT_SYS,  // special
    FMT_R,    // [opcode][dest][reg A][reg B]
    FMT_I,    // [opcode][dest][reg A][imm]
    FMT_R2,   // [opcode][dest][reg A]
    FMT_I2,   // [opcode][dest][imm]
    FMT_FR,   // [opcode][reg A][reg B]
    FMT_FI,   // [opcode][reg A][imm]
    FMT_B,    // [opcode][reg A][reg B][memory address label]
    FMT_M     // [opcode][dest/reg A][reg B][memory address label]
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

Instruction table[] = { // [5 opcode][3 type]
    // R-type (ALU = 000)
    {"ADD",   MAKE_OP(0b00000, 0b000), FMT_R},
    {"SUB",   MAKE_OP(0b00001, 0b000), FMT_R},
    {"MOV",   MAKE_OP(0b00010, 0b000), FMT_R2},
    {"AND",   MAKE_OP(0b00011, 0b000), FMT_R},
    {"OR",    MAKE_OP(0b00100, 0b000), FMT_R},
    {"XOR",   MAKE_OP(0b00101, 0b000), FMT_R},
    {"LSL",   MAKE_OP(0b00110, 0b000), FMT_R},
    {"LSR",   MAKE_OP(0b00111, 0b000), FMT_R},
    {"ROL",   MAKE_OP(0b01000, 0b000), FMT_R},
    {"ROR",   MAKE_OP(0b01001, 0b000), FMT_R},
    {"ASR",   MAKE_OP(0b01010, 0b000), FMT_R},
    {"NAND",  MAKE_OP(0b01011, 0b000), FMT_R},
    {"NOR",   MAKE_OP(0b01100, 0b000), FMT_R},
    {"XNOR",  MAKE_OP(0b01101, 0b000), FMT_R},
    {"NOT",   MAKE_OP(0b01110, 0b000), FMT_R},
    {"NEG",   MAKE_OP(0b01111, 0b000), FMT_R},

    // I-type (ALU imm = 001)
    {"ADDI",  MAKE_OP(0b00000, 0b001), FMT_I},
    {"SUBI",  MAKE_OP(0b00001, 0b001), FMT_I},
    {"MOVI",  MAKE_OP(0b00010, 0b001), FMT_I2},
    {"ANDI",  MAKE_OP(0b00011, 0b001), FMT_I},
    {"ORI",   MAKE_OP(0b00100, 0b001), FMT_I},
    {"XORI",  MAKE_OP(0b00101, 0b001), FMT_I},
    {"LSLI",  MAKE_OP(0b00110, 0b001), FMT_I},
    {"LSRI",  MAKE_OP(0b00111, 0b001), FMT_I},
    {"ROLI",  MAKE_OP(0b01000, 0b001), FMT_I},
    {"RORI",  MAKE_OP(0b01001, 0b001), FMT_I},
    {"ASRI",  MAKE_OP(0b01010, 0b001), FMT_I},
    {"NANDI", MAKE_OP(0b01011, 0b001), FMT_I},
    {"NORI",  MAKE_OP(0b01100, 0b001), FMT_I},
    {"XNORI", MAKE_OP(0b01101, 0b001), FMT_I},

    // B-type (Branch = 010)
    {"BEQ",   MAKE_OP(0b00000, 0b010), FMT_B},
    {"BNE",   MAKE_OP(0b00001, 0b010), FMT_B},
    {"BLTU",  MAKE_OP(0b00010, 0b010), FMT_B},
    {"BGTU",  MAKE_OP(0b00011, 0b010), FMT_B},
    {"BLT",   MAKE_OP(0b00100, 0b010), FMT_B},
    {"BGT",   MAKE_OP(0b00101, 0b010), FMT_B},
    {"BLEU",  MAKE_OP(0b00110, 0b010), FMT_B},
    {"BGEU",  MAKE_OP(0b00111, 0b010), FMT_B},
    {"BLE",   MAKE_OP(0b01000, 0b010), FMT_B},
    {"BGE",   MAKE_OP(0b01001, 0b010), FMT_B},

    // M-type (Memory = 011)
    {"LDR",   MAKE_OP(0b00000, 0b011), FMT_R},
    {"STR",   MAKE_OP(0b00001, 0b011), FMT_R},
    {"LDRI",  MAKE_OP(0b00010, 0b011), FMT_M},
    {"STRI",  MAKE_OP(0b00011, 0b011), FMT_M},
    
    // {"LDRB",  MAKE_OP(0b00100, 0b011), FMT_R}, Unsuported for now
    // {"STRB",  MAKE_OP(0b00101, 0b011), FMT_R}, Unsuported for now
    // {"LDRBI", MAKE_OP(0b00110, 0b011), FMT_M}, Unsuported for now
    // {"STRBI", MAKE_OP(0b00111, 0b011), FMT_M}, Unsuported for now
    
    {"JMP",   MAKE_OP(0b01000, 0b011), FMT_B},
    {"JMPA",  MAKE_OP(0b01001, 0b011), FMT_B},
    {"JMPR",  MAKE_OP(0b01010, 0b011), FMT_B},

    // System Control (111)
    {"LPT",   MAKE_OP(0b00000, 0b111), FMT_SYS}, // Load Page Table base from register

    // cache control
    {"INVALL",    MAKE_OP(0b00001, 0b111), FMT_SYS}, // Invalidate ALL cache lines (valid=0, dirty=0)
    {"WBALL",     MAKE_OP(0b00010, 0b111), FMT_SYS}, // Writeback ALL dirty lines to RAM (valid remains 1)
    {"FLUSHALL",  MAKE_OP(0b00011, 0b111), FMT_SYS}, // Writeback ALL dirty lines, then invalidate ALL

    {"INVLINE",   MAKE_OP(0b00100, 0b111), FMT_SYS}, // Invalidate line containing address in Rs
    {"WBLINE",    MAKE_OP(0b00101, 0b111), FMT_SYS}, // Writeback line containing address in Rs (if dirty)
    {"FLUSHLINE", MAKE_OP(0b00110, 0b111), FMT_SYS}, // Writeback line at address in Rs, then invalidate

    {"IREQ", MAKE_OP(0b00111, 0b111), FMT_SYS},

    {"HLT",   MAKE_OP(0b11111, 0b111), FMT_SYS}
};

const size_t num_of_instruction = sizeof(table) / sizeof(table[0]);

FILE *open_file(const char *program_name) {
    char file_path[256];

    char *dot = strrchr(program_name, '.');

    if (dot != NULL) {
        if (strcmp(dot, ".as") != 0) {
            fprintf(stderr, "Error: Invalid extension '%s'. Only '.as' files are allowed.\n", dot);
            return NULL;
        }
        snprintf(file_path, sizeof(file_path), "%s", program_name);
    } else {
        snprintf(file_path, sizeof(file_path), "%s.as", program_name);
    }

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

    char *dot = strrchr(program_name, '.');

    if (dot != NULL && strcmp(dot, ".as") == 0) {
        int base_length = (int)(dot - program_name);
        snprintf(file_path, sizeof(file_path), "%.*s.bin", base_length, program_name);
    } else {
        snprintf(file_path, sizeof(file_path), "%s.bin", program_name);
    }

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

int parse_immediate(const char *str, int *out_val) {
    char *endptr;
    errno = 0;
    long val;

    // Check for explicit binary prefix (0b / 0B)
    if ((str[0] == '0' && (str[1] == 'b' || str[1] == 'B')) ||
        (str[0] == '-' && str[1] == '0' && (str[2] == 'b' || str[2] == 'B'))) {
        
        int negative = (str[0] == '-');
        const char *bin_str = negative ? str + 3 : str + 2;

        val = strtol(bin_str, &endptr, 2); // Parse as Base 2
        if (negative) val = -val;
    } else {
        val = strtol(str, &endptr, 0);      // Auto-detect Base 10, 16, or 8
    }

    // Validation: check if parsing failed or left unparsed characters
    if (endptr == str || *endptr != '\0' || errno == ERANGE) {
        return 0; // Invalid number
    }

    *out_val = (int)val;
    return 1; // Success
}

int resolve_register(const char *arg, int addr) {
    if (!arg || arg[0] == '\0') {
        fprintf(stderr, "ERROR: Missing required register argument at address %d.\n", addr);
        exit(1);
    }

    if (arg[0] != 'R' && arg[0] != 'r') {
        fprintf(stderr, "ERROR: Expected register (starting with 'R'), got '%s' at address %d.\n", arg, addr);
        exit(1);
    }

    int reg_num;
    // Pass everything after 'R'/'r' to parse_immediate (handles hex, decimal, etc.)
    if (!parse_immediate(arg + 1, &reg_num)) {
        fprintf(stderr, "ERROR: Invalid register index in '%s' at address %d.\n", arg, addr);
        exit(1);
    }

    // Optional bounds check (e.g. 0-15 for 16 registers)
    if (reg_num < 0 || reg_num > 15) {
        fprintf(stderr, "ERROR: Register 'R%d' out of bounds (0-15) at address %d.\n", reg_num, addr);
        exit(1);
    }

    return reg_num;
}

// Returns 1 if the next valid line is a .WORD directive, 0 otherwise.
char *peek_next_line(FILE *fp, char *out_buffer, size_t buffer_size) {
    long original_pos = ftell(fp);
    char *found_ptr = NULL;

    while (fgets(out_buffer, buffer_size, fp)) {
        char *comment = strpbrk(out_buffer, "#;");
        if (comment) *comment = '\0';

        for (int i = 0; out_buffer[i] != '\0'; i++) {
            out_buffer[i] = toupper((unsigned char)out_buffer[i]);
        }

        char *ptr = out_buffer;
        while (isspace((unsigned char)*ptr)) ptr++;

        // Skip blank lines or comments
        if (*ptr == '\0') continue;

        // Found a valid line!
        found_ptr = ptr;
        break;
    }

    // Always restore original position
    fseek(fp, original_pos, SEEK_SET);
    return found_ptr;
}

void assembling(FILE *program, FILE *program_bin) {
    Label labels[MAX_LABELS];
    for (int i = 0; i < MAX_LABELS; i++)
        labels[i].address = -1;

    int label_count = 0;
    char line[256];
    int addr = 0;

    // --- Pass 1: Symbol Table Construction ---
    while (fgets(line, sizeof(line), program)) { 
        char *comment = strpbrk(line, "#;");
        if (comment) *comment = '\0';

        char line_copy[256];
        strcpy(line_copy, line);

        char *token = strtok(line, " \t\n,");
        if (token == NULL) continue;

        for (int i = 0; token[i]; i++) token[i] = toupper((unsigned char)token[i]);

        if (token[strlen(token) - 1] == ':') {
            token[strlen(token) - 1] = '\0';
            
            strcpy(labels[label_count].name, token);
            labels[label_count].address = addr;
            label_count++;

            token = strtok(NULL, " \t\r\n,");
            if (token == NULL) continue;
        }

        if (strcmp(token, ".WORD") == 0) {
            char temp_line[256];
            char *next_line = peek_next_line(program, temp_line, sizeof(temp_line));

            if (next_line != NULL && strstr(next_line, ".WORD") != NULL) {
                addr += 2;
                fgets(temp_line, sizeof(temp_line), program);
            } else {
                addr += 2;
            }

            continue;
        } else if (strcmp(token, ".STRING") == 0) {
            char *start_quote = strchr(line_copy, '"');
            char *end_quote = start_quote ? strrchr(start_quote + 1, '"') : NULL;

            if (start_quote && end_quote) {
                int str_len = (int)(end_quote - start_quote - 1);
                int total_bytes = str_len + 1;
                int words_needed = (total_bytes + 1) / 2;
                addr += (words_needed * 2);
            }
            continue;
        } else if (strcmp(token, "CALL") == 0) {
            addr += 4;
            continue;
        } else if (strcmp(token, ".SPACE") == 0) {
            char *bytes_str = strtok(NULL, " \t\r\n,");
            if (bytes_str != NULL) {
                int bytes = 0;
                if (parse_immediate(bytes_str, &bytes))
                    fprintf(stderr, "ERROR: parsing immediate \"%s\" failed on address %d.\n", bytes_str, addr);
                
                int words_32 = (bytes + 3) / 4;
                addr += words_32 * 2;
            }
            continue;
        }
        
        addr += 2;
    }
    
    rewind(program);
    addr = 0;

    // --- Pass 2: Code Generation & Debug Printing ---
    static uint16_t pending_word = 0;
    static int has_pending_word = 0;
    while (fgets(line, sizeof(line), program)) {
        char *comment = strpbrk(line, "#;");
        if (comment) *comment = '\0';

        // Preserve and trim clean assembly line for debug printing
        char raw_line[256];
        strcpy(raw_line, line);
        size_t raw_len = strlen(raw_line);
        while (raw_len > 0 && (raw_line[raw_len - 1] == '\n' || raw_line[raw_len - 1] == '\r' || 
                               raw_line[raw_len - 1] == ' '  || raw_line[raw_len - 1] == '\t')) {
            raw_line[--raw_len] = '\0';
        }

        for (int i = 0; line[i] != '\0'; i++) {
            line[i] = toupper((unsigned char)line[i]);
        }

        char line_copy[256];
        strcpy(line_copy, line);

        char *opcode = strtok(line, " \t\n,");
        if (opcode == NULL) continue;
        
        if (opcode[strlen(opcode) - 1] == ':') {
            opcode = strtok(NULL, " \t\r\n,"); // Get opcode after label
            if (opcode == NULL) continue;      // Standalone label line
        }

        uint32_t instruction = 0;
        
        if (strcmp(opcode, ".SPACE") == 0) {
            char *bytes_str = strtok(NULL, " \t\r\n,");
            if (bytes_str != NULL) {
                int bytes = 0;
                if (parse_immediate(bytes_str, &bytes))
                    fprintf(stderr, "ERROR: parsing immediate \"%s\" failed on address %d.\n", bytes_str, addr);

                int words_32 = (bytes + 3) / 4;
                for (int i = 0; i < words_32; i++) {
                    printf("0x%08x: %08x | %s\n", addr, instruction, raw_line);
                    fwrite(&instruction, sizeof(instruction), 1, program_bin);
                    addr += 2;
                }
            }
            continue;
        }

        if (strcmp(opcode, ".STRING") == 0) {
            char *start_quote = strchr(line_copy, '"');
            char *end_quote = start_quote ? strrchr(start_quote + 1, '"') : NULL;

            if (start_quote && end_quote) {
                uint8_t bytes[512];
                int total_bytes = 0;
                char *p = start_quote + 1;

                while (p < end_quote) {
                    if (*p == '\\' && (p + 1) < end_quote) {
                        p++;
                        switch (*p) {
                            case '0':  bytes[total_bytes++] = 0x00; break;
                            case 'n':  bytes[total_bytes++] = '\n'; break;
                            case 'r':  bytes[total_bytes++] = '\r'; break;
                            case 't':  bytes[total_bytes++] = '\t'; break;
                            case '\\': bytes[total_bytes++] = '\\'; break;
                            case '"':  bytes[total_bytes++] = '"';  break;
                            default:   bytes[total_bytes++] = *p;   break;
                        }
                    } else {
                        bytes[total_bytes++] = (uint8_t)(*p);
                    }
                    p++;
                }

                bytes[total_bytes++] = 0x00; // Null terminator

                int i = 0;
                while (i < total_bytes) {
                    uint32_t word_buffer = 0;
                    for (int b = 0; b < 4; b++) {
                        if (i < total_bytes) {
                            word_buffer |= ((uint32_t)bytes[i]) << (b * 8);
                            i++;
                        } else {
                            word_buffer |= ((uint32_t)0x00) << (b * 8);
                        }
                    }

                    fwrite(&word_buffer, sizeof(word_buffer), 1, program_bin);
                    printf("0x%08x: %08x | %s\n", addr, word_buffer, raw_line);
                    addr += 2;
                }
            }
            continue;
        }

        if (strcmp(opcode, ".WORD") != 0 && has_pending_word) {
            uint32_t flushed_inst = (uint32_t)pending_word;
            fwrite(&flushed_inst, sizeof(flushed_inst), 1, program_bin);
            printf("0x%08x: %08x | (.WORD padded flush)\n", addr, flushed_inst);
            
            has_pending_word = 0;
            addr += 2;
        }

        if (strcmp(opcode, ".WORD") == 0) {
            char *val_str = strtok(NULL, " \t\r\n,");
            if (val_str) {
                uint16_t raw_value1 = 0;
                int found_label = 0;

                for (int k = 0; k < label_count; k++) {
                    if (strcmp(val_str, labels[k].name) == 0) {
                        raw_value1 = labels[k].address;
                        found_label = 1;
                        break;
                    }
                }
                if (!found_label) {
                    raw_value1 = (uint16_t)strtoul(val_str, NULL, 0);
                }

                if (!has_pending_word) {
                    pending_word = raw_value1;
                    has_pending_word = 1;
                    continue;
                } else {
                    uint32_t instruction = ((uint32_t)raw_value1 << 16) | pending_word;
                    fwrite(&instruction, sizeof(instruction), 1, program_bin);
                    printf("0x%08x: %08x | %s\n", addr, instruction, raw_line);
                    
                    has_pending_word = 0;
                    addr += 2;
                    continue;
                }
            }
            continue;
        }

        int dest = 0, regA = 0, regB = 0, imm = 0;

        char *arg1 = strtok(NULL, " \t\n,");
        char *arg2 = strtok(NULL, " \t\n,");
        char *arg3 = strtok(NULL, " \t\n,");
        char *arg4 = strtok(NULL, " \t\n");
        
        if (strcmp(opcode, "INC") == 0 || strcmp(opcode, "INCF") == 0) {
            opcode = (char *)table[16].name;
            arg2 = arg1;
            arg3 = "1";
        } else if (strcmp(opcode, "DEC") == 0 || strcmp(opcode, "DECF") == 0) {
            opcode = (char *)table[17].name;
            arg2 = arg1;
            arg3 = "1";
        } else if (strcmp(opcode, "NOP") == 0) {
            opcode = (char *)table[0].name;
            arg1 = "R0";
            arg2 = "R0";
            arg3 = "R0";
        } else if (strcmp(opcode, "CALL") == 0) {
            instruction |= table[18].opcode | (0x000E << 8) | (((addr / 2) + 1) & 0xFFFF) << 16;
            fwrite(&instruction, sizeof(instruction), 1, program_bin);
            printf("0x%08x: %08x | %s (part 1)\n", addr, instruction, raw_line);
            addr += 2;
            opcode = (char *)table[44].name;
        } else if (strcmp(opcode, "RET") == 0) {
            opcode = (char *)table[46].name;
            arg1 = "R14";
        }

        size_t i;
        for (i = 0; i < num_of_instruction; i++) {
            if (strcmp(opcode, table[i].name) == 0) {
                instruction = (table[i].opcode);
                break;
            }
        }

        if (i == num_of_instruction) {
            fprintf(stderr, "ERROR 1: Unknown instruction '%s' on address %d.\n", opcode, addr);
            exit(1);
        }
        
        switch (table[i].format) {
            case FMT_SYS:
                if (strcmp(opcode, "HLT") == 0 || strcmp(opcode, "INVALL") == 0 || strcmp(opcode, "WBALL") == 0 || strcmp(opcode, "FLUSHALL") == 0 ||
                    strcmp(opcode, "HLT") == 0) {
                    if (arg1 != NULL || arg2 != NULL || arg3 != NULL || arg4 != NULL) {
                        fprintf(stderr, "ERROR on FMT_SYS: Unexpected arguments for '%s' at address %d.\n", opcode, addr);
                        exit(1);
                    }
                } else if (strcmp(opcode, "LPT") == 0 || strcmp(opcode, "INVLINE") == 0 || strcmp(opcode, "WBLINE") == 0 || strcmp(opcode, "FLUSHLINE") == 0) {
                    if (arg2 != NULL || arg3 != NULL || arg4 != NULL) {
                        fprintf(stderr, "ERROR on FMT_SYS: Unexpected extra arguments for '%s' at address %d.\n", opcode, addr);
                        exit(1);
                    }
                    regA = resolve_register(arg1, addr);
                    instruction |= (regA << 8);
                }
                break;

            case FMT_R:
                if (arg4 != NULL) {
                    fprintf(stderr, "ERROR on FMT_R: Unknown arg4 '%s' at address %d.\n", arg4, addr);
                    exit(1);
                }
                dest = resolve_register(arg1, addr);
                regA = resolve_register(arg2, addr);
                regB = resolve_register(arg3, addr);
                instruction |= (dest << 8) | (regA << 12) | (regB << 16);
                break;

            case FMT_I:
                if (arg4 != NULL) {
                    fprintf(stderr, "ERROR on FMT_I: Unknown arg4 '%s' at address %d.\n", arg4, addr);
                    exit(1);
                }
                dest = resolve_register(arg1, addr);
                regA = resolve_register(arg2, addr);

                if (!arg3 || !parse_immediate(arg3, &imm)) {
                    fprintf(stderr, "ERROR on FMT_I: Invalid or missing immediate '%s' at address %d.\n", arg3 ? arg3 : "NULL", addr);
                    exit(1);
                }
                instruction |= (dest << 8) | (regA << 12) | ((imm & 0xFFFF) << 16);
                break;

            case FMT_R2:
                if (arg3 != NULL || arg4 != NULL) {
                    fprintf(stderr, "ERROR on FMT_R2: Unexpected extra arguments at address %d.\n", addr);
                    exit(1);
                }
                dest = resolve_register(arg1, addr);
                regA = resolve_register(arg2, addr);
                instruction |= (dest << 8) | (regA << 12);
                break;

            case FMT_I2:
                if (arg3 != NULL || arg4 != NULL) {
                    fprintf(stderr, "ERROR on FMT_I2: Unexpected extra arguments at address %d.\n", addr);
                    exit(1);
                }
                dest = resolve_register(arg1, addr);
                imm = 0;
                if (arg2) {
                    int found_label = 0;
                    for (int k = 0; k < label_count; k++) {
                        if (strcmp(arg2, labels[k].name) == 0) {
                            imm = labels[k].address;
                            found_label = 1;
                            break;
                        }
                    }
                    if (!found_label) {
                        if (!parse_immediate(arg2, &imm)) {
                            fprintf(stderr, "ERROR on FMT_I2: Could not resolve label or immediate '%s' at address %d.\n", arg2, addr);
                            exit(1);
                        }
                    }
                } else {
                    fprintf(stderr, "ERROR on FMT_I2: Missing required immediate or label at address %d.\n", addr);
                    exit(1);
                }
                instruction |= (dest << 8) | ((imm & 0xFFFF) << 16);
                break;

            case FMT_FR:
                if (arg3 != NULL || arg4 != NULL) {
                    fprintf(stderr, "ERROR on FMT_FR: Unexpected extra arguments at address %d.\n", addr);
                    exit(1);
                }
                regA = resolve_register(arg1, addr);
                regB = resolve_register(arg2, addr);
                instruction |= (regA << 12) | (regB << 16);
                break;

            case FMT_FI:
                if (arg3 != NULL || arg4 != NULL) {
                    fprintf(stderr, "ERROR on FMT_FI: Unexpected extra arguments at address %d.\n", addr);
                    exit(1);
                }
                regA = resolve_register(arg1, addr);
                if (!arg2 || !parse_immediate(arg2, &imm)) {
                    fprintf(stderr, "ERROR on FMT_FI: Invalid or missing immediate '%s' at address %d.\n", arg2 ? arg2 : "NULL", addr);
                    exit(1);
                }
                instruction |= (regA << 12) | ((imm & 0xFFFF) << 16);
                break;

            case FMT_B:
                if (strcmp(opcode, "JMP") == 0) {
                    if (!arg1) {
                        fprintf(stderr, "ERROR on FMT_B (%s): Missing target address/label at address %d.\n", opcode, addr);
                        exit(1);
                    }

                    if (isdigit((unsigned char)arg1[0]) || arg1[0] == '-') {
                        if (!parse_immediate(arg1, &imm)) {
                            fprintf(stderr, "ERROR: Invalid immediate address '%s' at address %d.\n", arg1, addr);
                            exit(1);
                        }
                    } else {
                        int found = 0;
                        for (int k = 0; k < label_count; k++) {
                            if (strcmp(arg1, labels[k].name) == 0) {
                                imm = (labels[k].address / 2) - ((addr / 2) + 1);
                                found = 1;
                                break;
                            }
                        }
                        if (!found) {
                            fprintf(stderr, "ERROR: Could not resolve label '%s' at address %d.\n", arg1, addr);
                            exit(1);
                        }
                    }
                } else if (strcmp(opcode, "JMPR") == 0) {
                    regB = resolve_register(arg1, addr);
                } else if (strcmp(opcode, "JMPA") == 0) {
                    if (!arg1) {
                        fprintf(stderr, "ERROR on FMT_B (%s): Missing target address/label at address %d.\n", opcode, addr);
                        exit(1);
                    }

                    if (isdigit((unsigned char)arg1[0]) || arg1[0] == '-') {
                        if (!parse_immediate(arg1, &imm)) {
                            fprintf(stderr, "ERROR: Invalid immediate address '%s' at address %d.\n", arg1, addr);
                            exit(1);
                        }
                    } else {
                        int found = 0;
                        for (int k = 0; k < label_count; k++) {
                            if (strcmp(arg1, labels[k].name) == 0) {
                                imm = (labels[k].address / 2);
                                found = 1;
                                break;
                            }
                        }
                        if (!found) {
                            fprintf(stderr, "ERROR: Could not resolve label '%s' at address %d.\n", arg1, addr);
                            exit(1);
                        }
                    }
                } else {
                    if (arg1 == NULL || arg2 == NULL || arg3 == NULL) {
                        fprintf(stderr, "ERROR on FMT_B: Missing arguments at address %d.\n", addr);
                        exit(1);
                    }

                    if (arg4 != NULL) {
                        fprintf(stderr, "ERROR on FMT_B: Unexpected 4th argument '%s' at address %d.\n", arg4, addr);
                        exit(1);
                    }

                    regA = resolve_register(arg1, addr);
                    regB = resolve_register(arg2, addr);

                    int found = 0;
                    if (isdigit((unsigned char)arg3[0]) || arg3[0] == '-') {
                        if (parse_immediate(arg3, &imm)) {
                            found = 1;
                        }
                    } else {
                        for (int k = 0; k < label_count; k++) {
                            if (strcmp(arg3, labels[k].name) == 0) {
                                imm = (labels[k].address / 2) - ((addr / 2) + 1);
                                found = 1;
                                break;
                            }
                        }
                    }

                    if (!found) {
                        fprintf(stderr, "ERROR: Could not resolve label or immediate '%s' at address %d.\n", arg3, addr);
                        exit(1);
                    }
                }

                instruction |= (regA << 8) | (regB << 12) | ((imm & 0xFFFF) << 16);
                break;

            case FMT_M: {
                if (arg1 == NULL || arg2 == NULL || arg3 == NULL) {
                    fprintf(stderr, "ERROR on FMT_M: Missing arguments at address %d.\n", addr);
                    exit(1);
                }

                if (arg4 != NULL) {
                    fprintf(stderr, "ERROR on FMT_M: Unexpected 4th argument '%s' at address %d.\n", arg4, addr);
                    exit(1);
                }

                dest = resolve_register(arg1, addr);
                regA = resolve_register(arg2, addr);

                int found = 0;
                for (int k = 0; k < label_count; k++) {
                    if (strcmp(arg3, labels[k].name) == 0) {
                        imm = (labels[k].address / 2) - ((addr / 2) + 1);
                        found = 1;
                        break;
                    }
                }

                if (!found) {
                    if (!parse_immediate(arg3, &imm)) {
                        fprintf(stderr, "ERROR on FMT_M: Could not resolve label or immediate '%s' at address %d.\n", arg3, addr);
                        exit(1);
                    }
                }

                instruction |= (dest << 8) | (regA << 12) | ((imm & 0xFFFF) << 16);
                break;
            }

            default:
                fprintf(stderr, "ERROR: Unhandled instruction format %d at address %d.\n", table[i].format, addr);
                exit(1);
        }

        check_register_bounds(dest, regA, regB);

        fwrite(&instruction, sizeof(instruction), 1, program_bin);
        printf("0x%08x: %08x | %s\n", addr, instruction, raw_line);

        addr += 2;
    }

    if (has_pending_word) {
        uint32_t flushed_inst = (uint32_t)pending_word;
        fwrite(&flushed_inst, sizeof(flushed_inst), 1, program_bin);
        printf("0x%08x: %08x | (.WORD trailing flush)\n", addr, flushed_inst);
        addr += 2;
    }
}

int main(int argc, char * argv[]) {
    if (argc < 2) {
        printf("Usage: lang.exe <files...>\n");
        return 1;
    }

    const char *program_name = argv[1];

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
