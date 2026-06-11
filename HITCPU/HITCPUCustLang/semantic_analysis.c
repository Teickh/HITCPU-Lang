#include <stdio.h>
#include <stdlib.h>

#include "struct.h"

int create_new_scope(ScopeStack *scope_stack) {
    if (scope_stack->scope_count >= scope_stack->scope_capacity) {
        scope_stack->scope_capacity *= 2;

        SymbolTable *temp = realloc(scope_stack->scopes, scope_stack->scope_capacity * sizeof(SymbolTable));
        if (!temp) {
            printf("Scope tree scopes failed to reallocate. Out of memory");
            exit(1);
        }
        scope_stack->scopes = temp;
    }
    
    scope_stack->scopes[scope_stack->scope_count].symbol_count = 0;
    scope_stack->scopes[scope_stack->scope_count].symbol_capacity = 8;
    scope_stack->scopes[scope_stack->scope_count].symbols = malloc(8 * sizeof(Symbol));

    return scope_stack->scope_count++;
}

void add_symbol(SymbolTable *table, SymbolType type, int string_offset, TokenType data_type) {
    if (table->symbol_count >= table->symbol_capacity) {
        table->symbol_capacity *= 2;

        Symbol *temp = realloc(table->symbols, table->symbol_capacity * sizeof(Symbol));
        if (!temp) {
            printf("Current scope symbols failed to reallocate. Out of memory");
            exit(1);
        }
        table->symbols = temp;
    }

    table->symbols[table->symbol_count].type = type;
    table->symbols[table->symbol_count].string_offset = string_offset;
    table->symbols[table->symbol_count].data_type = data_type;
    table->symbol_count++;
}

int lookup_symbol(ScopeStack *scope_stack, int current_scope_index, int string_offset) {
    int index = current_scope_index;
    while (index != -1) {
        SymbolTable *table = &scope_stack->scopes[index];

        for (int i = 0; i < table->symbol_count; i++) {
            if (table->symbols[i].string_offset == string_offset) {
                return index;
            }
        }

        index--;
    }

    return -1;
}

void analyse_parameters(ASTTree *tree, int param_node_idx, ScopeStack *scope_stack) {
    if (param_node_idx == -1) return;

    ASTNode *param_node = &tree->nodes[param_node_idx];

    if (param_node->type == TOKEN_INT || param_node->type == TOKEN_CHAR) {
        if (param_node->left != -1) {
            ASTNode *ident_node = &tree->nodes[param_node->left];

            printf("Parameter: Registering input variable with string_offset %d. Line %d Column %d\n", 
                ident_node->data.string_offset,
                param_node->line,
                param_node->column);

            add_symbol(&scope_stack->scopes[scope_stack->stack], SYMBOL_LOCAL, ident_node->data.string_offset, param_node->type);
        }
    }
}

void analyse_block(ASTTree *tree, ASTNode *node, int current_node_index, ScopeStack *scope_stack, int current_scope_stack, int function_block, SymbolType symbol_type) {
    if (current_node_index == -1) return;

    int symbol_exists = 0;
    switch (node->type) {
        case TOKEN_FUNCTION: {
            add_symbol(&scope_stack->scopes[scope_stack->stack], SYMBOL_FUNCTION, tree->nodes[current_node_index].data.string_offset, tree->nodes[current_node_index].data.variable_type);
            create_new_scope(scope_stack);

            int param_list_idx = node->left;
            if (param_list_idx != -1) {
                ASTNode *param_node = &tree->nodes[param_list_idx];

                if (param_node->type == TOKEN_PARAM) {
                    for (int i = 0; i < param_node->data.param.param_count; i++) {
                        analyse_parameters(tree, param_node->data.param.param_indices[i], scope_stack);
                    }
                }
            }
            
            if (node->right != -1) {
                analyse_block(tree, &tree->nodes[node->right], node->right, scope_stack, scope_stack->stack, 1, symbol_type);
            }
            scope_stack->stack--;
            break;
        }
        
        case TOKEN_BLOCK: {
            if (function_block != 1) {
                create_new_scope(scope_stack);
            }

            int total = node->data.block.statement_count;
            int *stmts = node->data.block.statement_indices;
            for (int i = 0; i < total; i++) {
                analyse_block(tree, &tree->nodes[stmts[i]], stmts[i], scope_stack, scope_stack->stack, 0, symbol_type);
            }
            scope_stack->stack--;
            break;
        }

        case TOKEN_VAR_DECL: {
            int assign_node_index = node->right;

            if (assign_node_index != -1) {
                ASTNode *assign_node = &tree->nodes[assign_node_index];
                ASTNode *identification_node = &tree->nodes[assign_node->left];
                
                symbol_exists = lookup_symbol(scope_stack, current_scope_stack, identification_node->data.string_offset);
                if (symbol_exists != -1) {
                    printf("Error: variable has already been initialised. Line %d Column %d\n",
                        tree->nodes[node->left].line,
                        tree->nodes[node->left].column);
                    exit(1);
                }
            
                printf("Declaration: Registering new %s variable with string_offset %d. Line %d Column %d\n", 
                    token_type_to_string(identification_node->data.variable_type),
                    identification_node->data.string_offset,
                    identification_node->line,
                    identification_node->column);

                add_symbol(&scope_stack->scopes[scope_stack->stack], symbol_type, identification_node->data.string_offset, node->data.variable_type);

                if (assign_node->right != -1) {
                    analyse_block(tree, &tree->nodes[assign_node->right], assign_node->right, scope_stack, scope_stack->stack, 0, symbol_type);
                }
            } else {
                symbol_exists = lookup_symbol(scope_stack, current_scope_stack, tree->nodes[node->left].data.string_offset);
                if (symbol_exists != -1) {
                    printf("Error: variable has already been initialised. Line %d Column %d\n",
                        tree->nodes[node->left].line,
                        tree->nodes[node->left].column);
                    exit(1);
                }
                
                printf("Declaration: Registering new variable with string_offset %d. Line %d Column %d\n", 
                    tree->nodes[node->left].data.string_offset,
                    tree->nodes[node->left].line,
                    tree->nodes[node->left].column);

                add_symbol(&scope_stack->scopes[scope_stack->stack], symbol_type, tree->nodes[node->left].data.string_offset, node->data.variable_type);
            }
            
            break;
        }

        case TOKEN_ASSIGN: {
            int identification_node_index = node->left;
            ASTNode *identification_node = &tree->nodes[identification_node_index];

            printf("Assignment: Checking if variable %d exists in symbol table... Line %d Column %d\n",
                    identification_node->data.string_offset,
                    identification_node->line,
                    identification_node->column);

            symbol_exists = lookup_symbol(scope_stack, scope_stack->stack, identification_node->data.string_offset);

            if (symbol_exists == -1) {
                printf("Error: variable hasn't been initialised. Line %d Column %d",
                    tree->nodes[node->left].line,
                    tree->nodes[node->left].column);
                exit(1);
            }

            analyse_block(tree, &tree->nodes[node->right], node->right, scope_stack, scope_stack->stack, 0, symbol_type);
            break;
        }
        
        case TOKEN_IDENTIFIER: {
            printf("Variable Usage: Verifying variable %d exists to read its value. Line %d Column %d\n", 
                    node->data.string_offset,
                    node->line,
                    node->column);

            symbol_exists = lookup_symbol(scope_stack, scope_stack->stack, node->data.string_offset);
            
            if (symbol_exists == -1) {
                printf("Error: variable hasn't been initialised. Line %d Column %d",
                    node->line,
                    node->column);
                exit(1);
            }
            break;
        }

        default:
            if (node->left != -1)  analyse_block(tree, &tree->nodes[node->left], node->left, scope_stack, scope_stack->stack, 0, symbol_type);
            if (node->right != -1) analyse_block(tree, &tree->nodes[node->right], node->right, scope_stack, scope_stack->stack, 0, symbol_type);
            break;
    }
}

// analyse_block(ASTTree *tree, int current_node_idx) {
//     if (current_node_idx == -1) return;
    
//     int symbol_exists = 0;
//     ASTNode *current_node = &tree->nodes[current_node_idx];
//     switch (current_node->type) {
//         case TOKEN_VAR_DECL: {
//             int assign_node_index = current_node->right;

//             if (assign_node_index != -1) {
//                 ASTNode *assign_node = &tree->nodes[assign_node_index];
//                 ASTNode *identification_node = &tree->nodes[assign_node->left];
                
//                 symbol_exists = lookup_symbol(scope_stack, current_scope_stack, identification_node->data.string_offset);
//                 if (symbol_exists != -1) {
//                     printf("Error: variable has already been initialised. Line %d Column %d\n",
//                         tree->nodes[node->left].line,
//                         tree->nodes[node->left].column);
//                     exit(1);
//                 }
            
//                 printf("Declaration: Registering new %s variable with string_offset %d. Line %d Column %d\n", 
//                     token_type_to_string(identification_node->data.variable_type),
//                     identification_node->data.string_offset,
//                     identification_node->line,
//                     identification_node->column);

//                 add_symbol(&scope_stack->scopes[scope_stack->stack], symbol_type, identification_node->data.string_offset, node->data.variable_type);

//                 if (assign_node->right != -1) {
//                     analyse_block(tree, &tree->nodes[assign_node->right], assign_node->right, scope_stack, scope_stack->stack, 0, symbol_type);
//                 }
//             } else {
//                 symbol_exists = lookup_symbol(scope_stack, current_scope_stack, tree->nodes[node->left].data.string_offset);
//                 if (symbol_exists != -1) {
//                     printf("Error: variable has already been initialised. Line %d Column %d\n",
//                         tree->nodes[node->left].line,
//                         tree->nodes[node->left].column);
//                     exit(1);
//                 }
                
//                 printf("Declaration: Registering new variable with string_offset %d. Line %d Column %d\n", 
//                     tree->nodes[node->left].data.string_offset,
//                     tree->nodes[node->left].line,
//                     tree->nodes[node->left].column);

//                 add_symbol(&scope_stack->scopes[], symbol_type, tree->nodes[node->left].data.string_offset, node->data.variable_type);
//             }
            
//             break;
//         }

//         default:
//             break;
//     }
// }

void analyse(FileRegistry *registry, ASTTree *tree, ScopeStack *scope_stack) {
    int global_scope_idx = create_new_scope(scope_stack);

    int i = 0, j = 0;
    while (i < registry->function_count || j < registry->global_var_count) {
        if (j < registry->global_var_count) {
            // add_symbol(&scope_stack->scopes[scope_stack->stack], SYMBOL_GLOBAL, tree->nodes[registry->global_variables[j]].data.string_offset, tree->nodes[registry->global_variables[j]].data.variable_type);
            analyse_block(tree, &tree->nodes[registry->global_variables[j]], registry->global_variables[j], scope_stack, global_scope_idx, 0, SYMBOL_GLOBAL);
            j++;
        } else if (i < registry->function_count) {
            analyse_block(tree, &tree->nodes[registry->global_functions[i]], registry->global_functions[i], scope_stack, scope_stack->stack, 0, SYMBOL_LOCAL);
            i++;
        }
    }
}