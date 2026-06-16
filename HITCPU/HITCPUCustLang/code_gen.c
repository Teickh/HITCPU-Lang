#include <stdio.h>
#include <string.h>

#include "struct.h"

int create_instruction(InstructionStream *stream, int op) {
    if (stream->count >= stream->capacity) {
        stream->capacity = (stream->capacity == 0) ? 8 : stream->capacity * 2;

        AsmInstruction *temp = realloc(stream->data, stream->capacity * sizeof(AsmInstruction));
        if (!temp) {
            printf("Token realloc failed. Out of memory");
            exit(1);
        }
        
        stream->data = temp;
    }

    stream->data[stream->count].op = op;
    stream->data[stream->count].dest = -1;
    stream->data[stream->count].is_dead = 0;
    stream->data[stream->count].src1 = -1;
    stream->data[stream->count].src2_or_imm.imm = -1;
    stream->count++;
}

// void gen_node(InstructionStream *stream, ASTTree *tree, ASTNode *current_node, int current_node_idx) {
//     if (current_node->type == TOKEN_EOF) return;
    
//     switch (current_node->type)
//     {
//         case TOKEN_VAR_DECL:
            
//             break;
        
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

void gen_factor(ASTTree *tree, int current_node_idx) {
    ASTNode current_node = tree->nodes[current_node_idx];

    if (current_node.type == TOKEN_INT_LIT || current_node.type == TOKEN_CHAR_STRING) {
        gen_emit();
        return;
    }
    
    if (current_node.type == TOKEN_IDENTIFIER) {
        (*i)++;

        if (list->items[*i].type == TOKEN_LPAREN) {
            (*i)++;

            Token call_token = { .type = TOKEN_FUNCTION_CALL, .value_offset = current_token.value_offset };
            int call_node = create_node(tree, call_token, pool);

            int args_node = parse_argument_list(tree, list, pool, i);

            if (list->items[*i].type != TOKEN_RPAREN) {
                printf("Expected ')' after arguments at index %d\n", *i);
                exit(1);
            }
            (*i)++;

            safe_set_right(tree, call_node, args_node);

            return;
        } 
        else {
            return create_node(tree, current_token, pool);
        }
    }

    if (current_token.type == TOKEN_LPAREN) {
        (*i)++; // Consume '('
        int node = parse_expression(tree, list, i, pool); // Loop back up to top level expression!
        if (list->items[*i].type != TOKEN_RPAREN) { printf("Expected ')'\n"); exit(1); }
        (*i)++; // Consume ')'
        return node;
    }

    printf("Unexpected token in factor parsing at index %d\n", *i);
    exit(1);
}

void gen_term(ASTTree *tree, int current_node_idx) {
    gen_factor(tree, current_node_idx);

    while (tree->nodes[current_node_idx].type == TOKEN_MULTIPLY || tree->nodes[current_node_idx].type == TOKEN_DIVIDE) {
        gen_emit();
    }
}

void gen_expression(ASTTree *tree, int current_node_idx) {
    gen_term(tree, current_node_idx);

    while (tree->nodes[current_node_idx].type == TOKEN_PLUS || tree->nodes[current_node_idx].type == TOKEN_MINUS) {
        gen_emit();
    }
}

int parse_comparisons(ASTTree *tree, TokenList *list, int *i, StringPool *pool) {
    int current_node = parse_expression(tree, list, i, pool);

    while (list->items[*i].type == TOKEN_COMPARE_EQ || list->items[*i].type == TOKEN_NOT_EQ ||
           list->items[*i].type == TOKEN_GREATER_THAN || list->items[*i].type == TOKEN_LESS_THAN) {
        current_node = merge_node(tree, list, i, current_node, 'e', pool);
    }

    return current_node;
}

void gen_statement(ASTTree *tree, int current_node_idx) {
    if (current_node_idx == -1) return;

    ASTNode current_node = tree->nodes[current_node_idx];
    switch (current_node.type) {
        case TOKEN_VAR_DECL:
            int assign_node_idx = current_node.right;

            if (assign_node_idx != -1) {
                ASTNode *assign_node = &tree->nodes[assign_node_idx];
                ASTNode *identification_node = &tree->nodes[assign_node->left];

                gen_expression(tree, assign_node->right, pool);
            }
            
            break;
        
        default:
            break;
    }
}

void generate_code(FileRegistry *registry, ASTTree *tree, StringPool *pool, FILE *program_assembly, InstructionStream *stream) {
    for (int i = 0; i < registry->global_var_count; i++) {
        gen_statement(tree, i);
    }
    
        if (j < registry->global_var_count) {
            j++;
        }

    while (i < registry->function_count || j < registry->global_var_count) {
        if (i < registry->function_count) {
            gen_statement(tree, &i, &tree->nodes[registry->global_functions[i]], registry->global_functions[i]);
            i++;
        }
        
    }
}
