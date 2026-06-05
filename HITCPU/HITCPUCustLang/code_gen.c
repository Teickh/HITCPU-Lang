// // Addresses 0 – ??? (Global Data): let variables start here
// // Addresses ??? – END (Instructions): Reserved for compiled programs
// // Addresses 1023 – DOWN (The Stack): The Stack Pointer starts at 1023 and moves down toward 0.

// #include <stdio.h>
// #include <string.h>

// #include "struct.h"

// int create_instruction(InstructionStream *stream, int op) {
//     if (stream->count >= stream->capacity) {
//         stream->capacity = (stream->capacity == 0) ? 8 : stream->capacity * 2;

//         AsmInstruction *temp = realloc(stream->data, stream->capacity * sizeof(AsmInstruction));
//         if (!temp) {
//             printf("Token realloc failed. Out of memory");
//             exit(1);
//         }
        
//         stream->data = temp;
//     }

//     stream->data[stream->count].op = op;
//     stream->data[stream->count].dest = -1;
//     stream->data[stream->count].is_dead = 0;
//     stream->data[stream->count].src1 = -1;
//     stream->data[stream->count].src2_or_imm.imm = -1;
//     stream->count++;
// }

// void gen_node(InstructionStream *stream, ASTTree *tree, ASTNode *current_node, int current_node_idx) {
//     if (current_node->type == TOKEN_EOF) return;
    
//     switch (current_node->type)
//     {
//         case TOKEN_FUNCTION:
//             // handle parameters
//             ASTNode *param_node = &tree->nodes[current_node->left];

//             gen_node(stream, tree, param_node, current_node->left);

//             switch (current_node->left)
//             {
//             case constant expression:
//                 /* code */
//                 break;
            
//             default:
//                 break;
//             }
//             create_instruction(stream, )

//             // Traverse the function using recurse
//             while (current_node->left != -1) {
//                 gen_node(stream, tree, &tree->nodes[current_node->left], current_node->left);
//                 break;
//             }
            
//             if (current_node->right != -1) {
//                 gen_node(stream, tree, &tree->nodes[current_node->right], current_node->right);
//             }

//             // call and ret value
//             // emit_func
//             return;

//         case TOKEN_PARAM:
//             int *param_list = current_node->data.param.param_indices;
//             int total = current_node->data.param.param_count;

//             for (int i = 0; i < total; i++) {
//                 switch (tree->nodes[param_list[i]].type) {
//                     case TOKEN_INT_LIT: 
//                     case TOKEN_CHAR_STRING:
//                     case TOKEN_INT:
//                     case TOKEN_CHAR:
//                     case TOKEN_IF:
//                     case TOKEN_PRINT:
//                     case TOKEN_IDENTIFIER:
//                     case TOKEN_ASSIGN:
//                     case TOKEN_PLUS:
//                     case TOKEN_MINUS:
//                     case TOKEN_MULTIPLY:
//                     case TOKEN_DIVIDE:
//                     case TOKEN_COMPARE_EQ:
//                     case TOKEN_NOT_EQ:
//                     case TOKEN_GREATER_THAN:
//                     case TOKEN_LESS_THAN:
//                     case TOKEN_FUNCTION:
//                     case TOKEN_RETURN:
//                     case TOKEN_VAR_DECL:
//                     case TOKEN_UNKNOWN:
//                     case TOKEN_EOF:
//                     case TOKEN_BLOCK:
//                     case TOKEN_PARAM:
//                         break;

//                     default:
//                         printf("Unknown type %s. Line %d Column %d",
//                             token_type_to_string(tree->nodes[tree->count].type),
//                             tree->nodes[tree->count].line,
//                             tree->nodes[tree->count].column);
//                         exit(1);
//                 }
//                         create_instruction(stream, )
//                         /* code */
//                         break;
                    
//                     default:
//                         break;
//                 }
//             }
        
//             break;

//         case TOKEN_BLOCK:
//             // handle
//             // emit_block
//             return;

//         case TOKEN_IF:
//             // handle conditions
//             // emit_conditions
//             // recurse here to handle block
//             return;

//         case TOKEN_RETURN:
//             // handle return label/value
//             // return
//             return;
        
//         default:
//             break;
//     }

//     code_walk(tree, &tree->nodes[current_node->left]);
//     code_walk(tree, &tree->nodes[current_node->right]);
//     // then parent
    
//     // emit_expression
// }

// void generate_code(ProgramRegistry *registry, ASTTree *tree, StringPool *pool, FILE *program_assembly, InstructionStream *stream) {
//     int i = 0, j = 0;
//     while (i < registry->function_count || j < registry->global_var_count) {
//         if (i < registry->function_count) {
//             gen_node(stream, tree, &tree->nodes[registry->global_functions[i]], registry->global_functions[i]);
//             i++;
//         }
        
//         if (j < registry->global_var_count) {
//             j++;
//         }
//     }
// }
