#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "struct.h"

#define VREG_R0  1000
#define VREG_R1  1001
#define VREG_R14 1014
#define VREG_R15 1015

const char* get_op_name(int op) {
    // Array size is automatically bounded by OP_MAX_COUNT
    static const char* op_names[OP_MAX_COUNT] = {
        [OP_NONE]   = "NONE",

        // R-type
        [OP_ADD]    = "ADD",
        [OP_SUB]    = "SUB",
        [OP_MOV]    = "MOV",
        [OP_AND]    = "AND",
        [OP_OR]     = "OR",
        [OP_XOR]    = "XOR",
        [OP_INC]    = "INC",
        [OP_DEC]    = "DEC",
        [OP_LSL]    = "LSL",
        [OP_LSR]    = "LSR",
        [OP_ROL]    = "ROL",
        [OP_ROR]    = "ROR",
        [OP_ASR]    = "ASR",
        [OP_NAND]   = "NAND",
        [OP_NOR]    = "NOR",
        [OP_XNOR]   = "XNOR",
        [OP_NOT]    = "NOT",
        [OP_NEG]    = "NEG",
        [OP_RSB]    = "RSB",
        [OP_ADC]    = "ADC",
        [OP_SBC]    = "SBC",
        [OP_MUL]    = "MUL",
        [OP_DIV]    = "DIV",

        // I-type
        [OP_ADDI]   = "ADDI",
        [OP_SUBI]   = "SUBI",
        [OP_MOVI]   = "MOVI",
        [OP_ANDI]   = "ANDI",
        [OP_ORI]    = "ORI",
        [OP_XORI]   = "XORI",
        [OP_LSLI]   = "LSLI",
        [OP_LSRI]   = "LSRI",
        [OP_ROLI]   = "ROLI",
        [OP_RORI]   = "RORI",
        [OP_ASRI]   = "ASRI",
        [OP_NANDI]  = "NANDI",
        [OP_NORI]   = "NORI",
        [OP_XNORI]  = "XNORI",
        [OP_RSBI]   = "RSBI",
        [OP_ADCI]   = "ADCI",
        [OP_SBCI]   = "SBCI",

        // F-type
        [OP_ADDF]   = "ADDF",
        [OP_SUBF]   = "SUBF",
        [OP_ANDF]   = "ANDF",
        [OP_ORF]    = "ORF",
        [OP_XORF]   = "XORF",
        [OP_INCF]   = "INCF",
        [OP_DECF]   = "DECF",
        [OP_LSLF]   = "LSLF",
        [OP_LSRF]   = "LSRF",
        [OP_ROLF]   = "ROLF",
        [OP_RORF]   = "RORF",
        [OP_ASRF]   = "ASRF",
        [OP_NANDF]  = "NANDF",
        [OP_NORF]   = "NORF",
        [OP_XNORF]  = "XNORF",
        [OP_RSBF]   = "RSBF",
        [OP_ADCF]   = "ADCF",
        [OP_SBCF]   = "SBCF",

        [OP_CMP]    = "CMP",
        [OP_TST]    = "TST",
        [OP_TEQ]    = "TEQ",
        [OP_CMPI]   = "CMPI",
        [OP_TSTI]   = "TSTI",
        [OP_TEQI]   = "TEQI",

        // B-type
        [OP_JMP]    = "JMP",
        [OP_BEQ]    = "BEQ",
        [OP_BNE]    = "BNE",
        [OP_BGT]    = "BGT",
        [OP_BLT]    = "BLT",
        [OP_BGE]    = "BGE",
        [OP_BLE]    = "BLE",
        [OP_BCS]    = "BCS",
        [OP_BCC]    = "BCC",
        [OP_BMI]    = "BMI",
        [OP_BPL]    = "BPL",
        [OP_CALL]   = "CALL",
        [OP_RET]    = "RET",

        [OP_NOP]    = "NOP",
        [OP_HLT]    = "HLT",

        // Mem-Type
        [OP_LDR]    = "LDR",
        [OP_STR]    = "STR",
        
        [OP_LABEL]  = "LABEL"
    };

    // Bounds check to avoid out-of-bounds memory access
    if (op < 0 || op >= OP_MAX_COUNT || op_names[op] == NULL) {
        return "UNKNOWN";
    }

    return op_names[op];
}

void dump_stream_to_html(const char *filename, InstructionStream *stream, SymbolLists *sym_lists, StringPool *string_pool) {
    FILE *html = fopen(filename, "w");
    if (!html) {
        printf("Error: Could not open debug file %s\n", filename);
        return;
    }

    // Write HTML Boilerplate and basic modern CSS styling
    fprintf(html, "<!DOCTYPE html>\n<html>\n<head>\n<title>IR Stream Debug</title>\n");
    fprintf(html, "<style>\n");
    fprintf(html, "  body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #1e1e1e; color: #d4d4d4; padding: 20px; }\n");
    fprintf(html, "  h2 { color: #4fc1ff; border-bottom: 2px solid #333; padding-bottom: 10px; }\n");
    fprintf(html, "  table { width: 100%%; border-collapse: collapse; margin-top: 20px; box-shadow: 0 4px 6px rgba(0,0,0,0.3); }\n");
    fprintf(html, "  th, td { padding: 12px; text-align: left; border-bottom: 1px solid #3c3c3c; }\n");
    fprintf(html, "  th { background-color: #252526; color: #85e89d; font-weight: 600; }\n");
    fprintf(html, "  tr:hover { background-color: #2a2d2e; }\n");
    fprintf(html, "  .reg { color: #ce9178; font-weight: bold; }\n");
    fprintf(html, "  .imm { color: #b5cea8; }\n");
    fprintf(html, "  .op { color: #569cd6; font-weight: bold; }\n");
    fprintf(html, "  .mem { color: #c586c0; }\n");
    fprintf(html, "  .none { color: #6a9955; font-style: italic; }\n");
    fprintf(html, "</style>\n</head>\n<body>\n");

    fprintf(html, "<h2>Generated Instruction Stream Debug</h2>\n");
    fprintf(html, "<p>Total Instructions: <strong>%d</strong> | Capacity: <strong>%d</strong></p>\n", stream->count, stream->capacity);
    
    fprintf(html, "<table>\n<tr>\n");
    fprintf(html, "  <th>#</th>\n  <th>Opcode</th>\n  <th>Destination</th>\n  <th>Src 1</th>\n  <th>Src 2 / Imm / Offset</th>\n");
    fprintf(html, "</tr>\n");

    // Loop through and format each instruction
    for (int i = 0; i < stream->count; i++) {
        AsmInstruction instr = stream->data[i];
        
        fprintf(html, "<tr>\n");
        fprintf(html, "  <td>%d</td>\n", i);
        fprintf(html, "  <td class='op'>%s</td>\n", get_op_name(instr.op));
        
        // Dest
        if (instr.dest >= 1000) fprintf(html, "<td class='reg'>R%d</td>\n", instr.dest - 1000);
        else fprintf(html, "<td class='reg'>vreg_%d</td>\n", instr.dest);

        // Src1
        if (instr.src1 >= 1000) fprintf(html, "<td class='reg'>R%d</td>\n", instr.src1 - 1000);
        else if (instr.src1 != -1) fprintf(html, "<td class='reg'>vreg_%d</td>\n", instr.src1);
        else fprintf(html, "<td class='none'>-</td>\n");

        // Src2 / Imm / Offset variants based on your emit logic
        // (Assuming you're tracking the type or keeping the structure properties)
        if (instr.op == OP_MOVI || instr.op == OP_SUBI || instr.op == OP_ADDI) {
            fprintf(html, "  <td class='imm'>#%d (Imm)</td>\n", instr.src2_or_imm.imm);
        } else if (instr.op == OP_STR || instr.op == OP_LABEL || instr.op == OP_LDR ||
                   instr.op == OP_BGT || instr.op == OP_BLE || instr.op == OP_BLT ||
                   instr.op == OP_BGE || instr.op == OP_BEQ || instr.op == OP_BNE ||
                   instr.op == OP_CALL) {
            // --- VARIANT LOOKUP LOGIC ---
            int target_offset = instr.src2_or_imm.offset;
            const char *var_name = NULL;

            // --- CHECK IF IT IS A SYSTEM LABEL FIRST ---
            if (instr.op == OP_LABEL || instr.op == OP_BLE || instr.op == OP_BNE || 
                instr.op == OP_BEQ || instr.op == OP_BLT || instr.op == OP_BGT) {
                
                if (string_pool && target_offset < string_pool->size) {
                    var_name = &string_pool->data[target_offset];
                }
            }
            // --- HISTORICAL SCOPE SEARCH LOOP ---
            else if (sym_lists && string_pool) {
                // Loop through every single historical SymbolTable scope
                for (int t = 0; t < sym_lists->count; t++) {
                    SymbolTable *current_table = &sym_lists->historical_scopes[t];
                    
                    // Loop through the symbols in this specific scope table
                    for (int s = 0; s < current_table->symbol_count; s++) {
                        if (current_table->symbols[s].string_offset == target_offset) {
                            // Found it! Grab the pointer from the string pool
                            var_name = &string_pool->data[target_offset];
                            break; 
                        }
                    }
                    if (var_name) break; // Break out of outer loop if found
                }
            }
            
            if (target_offset == VREG_R15) {
                var_name = "stack";
            }

            if (var_name) {
                fprintf(html, "  <td class='mem'>%s (Mem)</td>\n", var_name);
            } else {
                // Fallback just in case a temporary offset isn't in the symbol lists
                fprintf(html, "  <td class='mem'>offset_%d (Mem)</td>\n", target_offset);
            }
            // -----------------------------
        } else {
            // standard 3-address operations like ADD, SUB, etc.
            if (instr.src2_or_imm.src2 != -1) {
                fprintf(html, "  <td class='reg'>vreg_%d</td>\n", instr.src2_or_imm.src2);
            } else {
                fprintf(html, "  <td class='none'>-</td>\n");
            }
        }
        
        fprintf(html, "</tr>\n");
    }

    fprintf(html, "</table>\n</body>\n</html>\n");
    fclose(html);
    printf("Debug HTML generated successfully: %s\n", filename);
}

int next_vreg = 0;
int then_label_counter = 0;
int else_label_counter = 0;
int end_label_counter = 0;

int fresh_reg() {
    return next_vreg++; // just keep handing out new "registers"
}

int string_pool_insert(StringPool *pool, char *label_buffer) {
    int len = strlen(label_buffer) + 1;
    if (pool->size + len >= pool->capacity) {
        pool->capacity *= 2;

        char *temp = realloc(pool->data, pool->capacity);
        if (!temp) {
            printf("Out of memory");
            exit(1);
        }
        pool->data = temp;
    }
    
    int inserted_offset = pool->size;
    strcpy(&pool->data[inserted_offset], label_buffer);
    pool->size += len;
    return inserted_offset;
}

int generate_unique_else_label(StringPool *pool) {
    char label_buffer[32];
    // Generate a uniquely numbered label name
    sprintf(label_buffer, ".L_ELSE_%d", else_label_counter++);
    
    // Insert into your string pool and return the offset/index
    return string_pool_insert(pool, label_buffer); 
}

int generate_unique_then_label(StringPool *pool) {
    char label_buffer[32];
    // Generate a uniquely numbered label name
    sprintf(label_buffer, ".L_THEN_%d", then_label_counter++);
    
    // Insert into your string pool and return the offset/index
    return string_pool_insert(pool, label_buffer); 
}

int generate_unique_end_label(StringPool *pool) {
    char label_buffer[32];
    // Generate a uniquely numbered label name
    sprintf(label_buffer, ".L_END_%d", end_label_counter++);
    
    // Insert into your string pool and return the offset/index
    return string_pool_insert(pool, label_buffer); 
}

// int peek_left_child(ASTTree *tree, int current_idx, int offset) {
//     int target = current_idx + offset;

//     if (target >= tree->count || target < 0) {
//         return TOKEN_EOF;
//     }
    
//     return tree->nodes[target].left;
// }

// int peek_right_child(ASTTree *tree, int current_idx, int offset) {
//     int target = current_idx + offset;

//     if (target >= tree->count || target < 0) {
//         return TOKEN_EOF;
//     }
    
//     return tree->nodes[target].right;
// }

int emit(InstructionStream *stream, int op, int dest, int src1, int src2_or_imm, int is_imm, int is_mem) {
    if (stream->count >= stream->capacity) {
        stream->capacity *= 2;

        AsmInstruction *temp = realloc(stream->data, stream->capacity * sizeof(AsmInstruction));
        if (!temp) {
            printf("Token realloc failed. Out of memory");
            exit(1);
        }
        
        stream->data = temp;
    }

    AsmInstruction *instr = &stream->data[stream->count];
    instr->op = op;
    instr->dest = dest;
    instr->src1 = src1;
    instr->is_dead = 0;

    if (is_imm && !is_mem)
        instr->src2_or_imm.imm = src2_or_imm;
    else if (!is_imm && !is_mem)
        instr->src2_or_imm.src2 = src2_or_imm;
    else if (!is_imm && is_mem)
        instr->src2_or_imm.offset = src2_or_imm;
    
    return stream->count++;
}

int gen_expression(ASTTree *tree, int current_node_idx, InstructionStream *stream) {
    if (current_node_idx == -1) return -1;

    ASTNode *current_node = &tree->nodes[current_node_idx];
    switch (current_node->type) {
        // case TOKEN_BLOCK: {
        //     for (int i = 0; i < current_node->data.block.statement_count; i++) {
        //         gen_expression(tree, current_node->data.block.statement_indices[i], stream, symbol_lists);
        //     }
        // }

        case TOKEN_INT_LIT:{
            int dest = fresh_reg();
            emit(stream, OP_MOVI, dest, -1, current_node->data.number_value, 1, 0);
            return dest;
        }
        
        case TOKEN_IDENTIFIER: {
            int dest = fresh_reg();
            emit(stream, OP_LDR, dest, -1, current_node->data.string_offset, 0, 1);
            return dest;
        }

        default: return -1;
    }
}

void gen_condition(ASTTree *tree, int node_idx, InstructionStream *stream, SymbolLists *symbol_lists, StringPool *pool, int true_label, int false_label) {
    if (node_idx == -1) return;
    ASTNode *node = &tree->nodes[node_idx];

    // --- OR SHORT CIRCUIT ---
    if (node->type == TOKEN_OR) {
        // If Left side is true, we jump straight to true_label (Short-Circuit!)
        // If Left side is false, it falls through to evaluate the Right side.
        int next_cond_label = generate_unique_then_label(pool); 
        
        gen_condition(tree, node->left, stream, symbol_lists, pool, true_label, next_cond_label);
        
        emit(stream, OP_LABEL, -1, -1, next_cond_label, 0, 1);
        gen_condition(tree, node->right, stream, symbol_lists, pool, true_label, false_label);
        return;
    }

    // --- AND SHORT CIRCUIT ---
    if (node->type == TOKEN_AND) {
        // If Left side is false, we jump straight to false_label (Short-Circuit!)
        // If Left side is true, it falls through to evaluate the Right side.
        int next_cond_label = generate_unique_then_label(pool);
        
        gen_condition(tree, node->left, stream, symbol_lists, pool, next_cond_label, false_label);
        
        emit(stream, OP_LABEL, -1, -1, next_cond_label, 0, 1);
        gen_condition(tree, node->right, stream, symbol_lists, pool, true_label, false_label);
        return;
    }

    // --- BASE COMPARISONS (>, <, ==, !=, etc.) ---
    if (node->type >= TOKEN_COMPARE_EQ && node->type <= TOKEN_LESS_THAN_OR_EQ) {
        // Evaluate numerical expressions to get raw data registers
        int left_reg = gen_expression(tree, node->left, stream);
        int right_reg = gen_expression(tree, node->right, stream);
        
        OpcodeType op;
        switch (node->type) {
            case TOKEN_COMPARE_EQ:   op = OP_BEQ; break;
            case TOKEN_NOT_EQ:       op = OP_BNE; break;
            case TOKEN_GREATER_THAN: op = OP_BGT; break;
            case TOKEN_GREATER_THAN_OR_EQ: op = OP_BGE; break;
            case TOKEN_LESS_THAN:    op = OP_BLT; break;
            case TOKEN_LESS_THAN_OR_EQ: op = OP_BLE; break;
            default: printf("Unhandle comparison"); exit(1);
        }
        
        // If condition passes, jump to true branch
        emit(stream, op, left_reg, right_reg, true_label, 0, 1);
        // Otherwise, jump explicitly to the false destination path
        emit(stream, OP_JMP, -1, -1, false_label, 0, 1);
        return;
    }

    // Fallback: If it's a naked identifier/boolean literal (e.g., if (flag))
    int reg = gen_expression(tree, node_idx, stream);
    emit(stream, OP_BNE, reg, 0, true_label, 0, 1); // If reg != 0, True
    emit(stream, OP_JMP, -1, -1, false_label, 0, 1);
}

void gen_node(ASTTree *tree, int current_node_idx, InstructionStream *stream, SymbolLists *symbol_lists, StringPool *pool) {
    if (current_node_idx == -1) return;

    ASTNode *current_node = &tree->nodes[current_node_idx];
    switch (current_node->type) {
        case TOKEN_FUNCTION_CALL:{
            ASTNode *param_list = &tree->nodes[current_node->right];
            for (int i = 0; i < param_list->data.param.param_count; i++) {
                ASTNode *param = &tree->nodes[param_list->data.param.param_indices[i]];
                if (i < 7) {
                    if (param->type == TOKEN_IDENTIFIER) {
                        emit(stream, OP_LDR, i, -1, param->data.number_value, 1, 0);
                    } else {
                        emit(stream, OP_MOVI, i, -1, param->data.number_value, 1, 0);
                    }
                } else {
                    emit(stream, OP_MOVI, i, -1, param->data.number_value, 1, 0);
                    emit(stream, OP_STR, i, -1, VREG_R15, 1, 0);
                }
            }

            emit(stream, OP_CALL, -1, -1, current_node->data.string_offset, 0, 1);
            break;
        }

        case TOKEN_FUNCTION: {
            // Label
            emit(stream, OP_LABEL, -1, -1, current_node->data.function.name_string_offset, 0, 1);

            // Prologue
            int stack_size = 0;
            ASTNode *param_list = &tree->nodes[current_node->left];
            for (int i = 0; i < param_list->data.param.param_count; i++) {
                ASTNode *param = &tree->nodes[param_list->data.param.param_indices[i]];
                switch (param->data.variable_type) {
                    case TOKEN_CHAR: stack_size += 1; break;
                    case TOKEN_INT:  stack_size += 2; break;
                    default:         stack_size += 2; break;
                }
            }
            ASTNode *block = &tree->nodes[current_node->right];
            for (int i = 0; i < block->data.block.statement_count; i++) {
                ASTNode *stmt = &tree->nodes[block->data.block.statement_indices[i]];
                if (stmt->type == TOKEN_VAR_DECL) {
                    switch (stmt->data.variable_type) {
                        case TOKEN_CHAR: stack_size += 1; break;
                        case TOKEN_INT:  stack_size += 2; break;
                        default:         stack_size += 2; break;
                    }
                }
            }
            emit(stream, OP_SUBI, VREG_R15, VREG_R15, stack_size, 1, 0);

            // parameter
            for (int i = 0; i < param_list->data.param.param_count; i++) {
                ASTNode *param = &tree->nodes[param_list->data.param.param_indices[i]];
                if (i < 7) {
                    emit(stream, OP_STR, i, -1, tree->nodes[param->left].data.string_offset, 1, 0);
                }
            }

            // body
            for (int i = 0; i < block->data.block.statement_count; i++) {
                gen_node(tree, block->data.block.statement_indices[i], stream, symbol_lists, pool);
            }
            
            // epilogue
            emit(stream, OP_ADDI, VREG_R15, VREG_R15, stack_size, 1, 0);
            emit(stream, OP_RET, -1, -1, -1, 0, 0);
            break;
        }

        case TOKEN_IF: {
            int then_label = generate_unique_then_label(pool);
            int else_label = generate_unique_else_label(pool);
            int end_label  = generate_unique_end_label(pool);

            IfStatement if_data = current_node->data.if_statement;

            // comparisons
            gen_condition(tree, if_data.condition_idx, stream, symbol_lists, pool, then_label, else_label);

            // then body
            emit(stream, OP_LABEL, -1, -1, then_label, 0, 1);
            if (if_data.true_block_idx != -1) {
                ASTNode *true_block = &tree->nodes[if_data.true_block_idx];
                for (int i = 0; i < true_block->data.block.statement_count; i++) {
                    gen_node(tree, true_block->data.block.statement_indices[i], stream, symbol_lists, pool);
                }
            }
            emit(stream, OP_JMP, -1, -1, end_label, 0, 1);

            // else body
            emit(stream, OP_LABEL, -1, -1, else_label, 0, 1);
            if (if_data.false_block_idx != -1) {
                ASTNode *false_block = &tree->nodes[if_data.false_block_idx];
                
                if (false_block->type == TOKEN_IF) {
                    gen_node(tree, if_data.false_block_idx, stream, symbol_lists, pool);
                } else {
                    for (int i = 0; i < false_block->data.block.statement_count; i++) {
                        gen_node(tree, false_block->data.block.statement_indices[i], stream, symbol_lists, pool);
                    }
                }
            }

            // end label
            emit(stream, OP_LABEL, -1, -1, end_label, 0, 1);
            break;
        }

        case TOKEN_VAR_DECL: {
            int assign_node_idx = current_node->right;

            if (assign_node_idx != -1) {
                ASTNode *assign_node = &tree->nodes[assign_node_idx];
                ASTNode *identification_node = &tree->nodes[assign_node->left];

                int result = gen_expression(tree, assign_node->right, stream);

                emit(stream, OP_STR, result, -1, identification_node->data.string_offset, 0, 1);
            }
            
            break;
        }

        case TOKEN_ASSIGN: {
            ASTNode *ident_node = &tree->nodes[current_node->left];
            int result = gen_expression(tree, current_node->right, stream);
            emit(stream, OP_STR, result, -1, ident_node->data.string_offset, 0, 1);
            break;
        }

        case TOKEN_RETURN: {
            int result = gen_expression(tree, current_node->left, stream);
            emit(stream, OP_MOV, REG_R1, result, -1, 0, 0);
            break;
        }

        default:
            break;
    }
}

void generate_code(FileRegistry *registry, ASTTree *tree, StringPool *pool, InstructionStream *stream, SymbolLists *symbol_lists) {
    for (int i = 0; i < registry->global_var_count; i++) {
        gen_node(tree, registry->global_variables[i], stream, symbol_lists, pool);
    }

    for (int i = 0; i < registry->function_count; i++) {
        gen_node(tree, registry->global_functions[i], stream, symbol_lists, pool);
    }
}
