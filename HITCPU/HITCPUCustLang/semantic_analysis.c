#include <stdio.h>
#include <stdlib.h>

#include "struct.h"

int create_new_scope(ScopeTree *scope_tree) {
    if (scope_tree->scope_count >= scope_tree->scope_capacity) {
        scope_tree->scope_capacity = (scope_tree->scope_capacity == 0) ? 8 : scope_tree->scope_capacity *2;

        Scope *temp = realloc(scope_tree->scopes, scope_tree->scope_capacity * sizeof(Scope));
        if (!temp) {
            printf("Scope tree scopes failed to reallocate. Out of memory");
            exit(1);
        }
        scope_tree->scopes = temp;
    }
    
    scope_tree->scopes[scope_tree->scope_count].symbols = NULL;
    scope_tree->scopes[scope_tree->scope_count].symbol_count = 0;
    scope_tree->scopes[scope_tree->scope_count].symbol_capacity = 0;
    scope_tree->scopes[scope_tree->scope_count].parent_scope_idx = -1;

    return scope_tree->scope_count++;
}

void add_symbol(ScopeTree *scope_tree, int current_scope_index, SymbolType type, int string_offset, TokenType data_type) {
    Scope *current_scope = &scope_tree->scopes[current_scope_index];

    if (current_scope->symbol_count >= current_scope->symbol_capacity) {
        current_scope->symbol_capacity = (current_scope->symbol_capacity == 0) ? 8 : current_scope->symbol_capacity * 2;

        Symbol *temp = realloc(current_scope->symbols, current_scope->symbol_capacity * sizeof(Symbol));
        if (!temp) {
            printf("Current scope symbols failed to reallocate. Out of memory");
            exit(1);
        }
        current_scope->symbols = temp;
    }

    current_scope->symbols[current_scope->symbol_count].type = type;
    current_scope->symbols[current_scope->symbol_count].string_offset = string_offset;
    current_scope->symbols[current_scope->symbol_count].data_type = data_type; // Unsure on what to do
    current_scope->symbols[current_scope->symbol_count].stack_slot = current_scope->symbol_count;

    current_scope->symbol_count++;
}

int lookup_symbol(ScopeTree *scope_tree, int current_scope_index, int string_offset) {
    int index = current_scope_index;
    while (index != -1) {
        Scope *scope = &scope_tree->scopes[index];

        for (int i = 0; i < scope->symbol_count; i++) {
            if (scope->symbols[i].string_offset == string_offset) {
                return index;
            }
        }
        index = scope->parent_scope_idx;
    }

    return -1;
}

void analyse_parameters(ASTTree *tree, int param_node_idx, ScopeTree *scope_tree, int function_scope_idx) {
    if (param_node_idx == -1) return;

    ASTNode *param_node = &tree->nodes[param_node_idx];

    if (param_node->type == TOKEN_INT || param_node->type == TOKEN_CHAR) {
        if (param_node->left != -1) {
            ASTNode *ident_node = &tree->nodes[param_node->left];

            printf("Parameter: Registering input variable with string_offset %d. Line %d Column %d\n", 
                ident_node->data.string_offset,
                param_node->line,
                param_node->column);

            add_symbol(scope_tree, function_scope_idx, SYMBOL_LOCAL, ident_node->data.string_offset, param_node->type);
        }
    }
}

void analyse_block(ASTTree *tree, ASTNode *node, int current_node_index, ScopeTree *scope_tree, int current_scope_index) {
    if (current_node_index == -1) return;

    int symbol_exists = 0;
    switch (node->type) {
        case TOKEN_FUNCTION: {
            int function_scope_idx = create_new_scope(scope_tree);
            scope_tree->scopes[function_scope_idx].parent_scope_idx = current_scope_index;

            int param_list_idx = node->left;
            if (param_list_idx != -1) {
                ASTNode *param_node = &tree->nodes[param_list_idx];

                if (param_node->type == TOKEN_PARAM) {
                    for (int i = 0; i < param_node->data.param.param_count; i++) {
                        analyse_parameters(tree, param_node->data.param.param_indices[i], scope_tree, function_scope_idx);
                    }
                }
            }
            
            if (node->right != -1) {
                analyse_block(tree, &tree->nodes[node->right], node->right, scope_tree, function_scope_idx);
            }
            break;
        }
        
        case TOKEN_BLOCK: {
            int new_scope_idx = create_new_scope(scope_tree);

            scope_tree->scopes[new_scope_idx].parent_scope_idx = current_scope_index;

            int total = node->data.block.statement_count;
            int *stmts = node->data.block.statement_indices;
            for (int i = 0; i < total; i++) {
                analyse_block(tree, &tree->nodes[stmts[i]], stmts[i], scope_tree, new_scope_idx);
            }
            break;
        }

        case TOKEN_VAR_DECL: {
            symbol_exists = lookup_symbol(scope_tree, current_scope_index, tree->nodes[node->left].data.string_offset);
            if (symbol_exists != -1) {
                printf("Error: variable has already been initialised. Line %d Column %d\n",
                    tree->nodes[node->left].line,
                    tree->nodes[node->left].column);
                exit(1);
            }

            int assign_node_index = node->right;

            if (assign_node_index != -1) {
                ASTNode *assign_node = &tree->nodes[assign_node_index];
                ASTNode *identification_node = &tree->nodes[assign_node->left];
            
                printf("Declaration: Registering new INT variable with string_offset %d. Line %d Column %d\n", 
                    identification_node->data.string_offset,
                    identification_node->line,
                    identification_node->column);

                add_symbol(scope_tree, current_scope_index, SYMBOL_LOCAL, identification_node->data.string_offset, node->data.variable_type);

                if (assign_node->right != -1) {
                    analyse_block(tree, &tree->nodes[assign_node->right], assign_node->right, scope_tree, current_scope_index);
                }
            } else {
                add_symbol(scope_tree, current_scope_index, SYMBOL_LOCAL, tree->nodes[node->left].data.string_offset, node->data.variable_type);
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

            symbol_exists = lookup_symbol(scope_tree, current_scope_index, identification_node->data.string_offset);

            if (symbol_exists == -1) {
                printf("Error: variable hasn't been initialised. Line %d Column %d",
                    tree->nodes[node->left].line,
                    tree->nodes[node->left].column);
                exit(1);
            }

            analyse_block(tree, &tree->nodes[node->right], node->right, scope_tree, current_scope_index);
            break;
        }
        
        case TOKEN_IDENTIFIER: {
            printf("Variable Usage: Verifying variable %d exists to read its value. Line %d Column %d\n", 
                    node->data.string_offset,
                    node->line,
                    node->column);

            symbol_exists = lookup_symbol(scope_tree, current_scope_index, node->data.string_offset);
            
            if (symbol_exists == -1) {
                printf("Error: variable hasn't been initialised. Line %d Column %d",
                    node->line,
                    node->column);
                exit(1);
            }
            break;
        }

        default:
            if (node->left != -1)  analyse_block(tree, &tree->nodes[node->left], node->left, scope_tree, current_scope_index);
            if (node->right != -1) analyse_block(tree, &tree->nodes[node->right], node->right, scope_tree, current_scope_index);
            break;
    }
}

void analyse(ProgramRegistry *registry, ASTTree *tree, ScopeTree *scope_tree, StringPool *pool) {
    int global_scope_idx = create_new_scope(scope_tree);

    int i = 0, j = 0;
    while (i < registry->function_count || j < registry->global_var_count) {
        if (j < registry->global_var_count) {
            add_symbol(scope_tree, global_scope_idx, SYMBOL_GLOBAL, tree->nodes[registry->global_variables[j]].data.string_offset, tree->nodes[registry->global_variables[j]].data.variable_type);
            analyse_block(tree, &tree->nodes[registry->global_functions[i]], registry->global_functions[i], scope_tree, global_scope_idx);
            j++;
        }
        
        if (i < registry->function_count) {
            add_symbol(scope_tree, global_scope_idx, SYMBOL_FUNCTION, tree->nodes[registry->global_functions[i]].data.string_offset, tree->nodes[registry->global_functions[i]].data.variable_type);
            analyse_block(tree, &tree->nodes[registry->global_functions[i]], registry->global_functions[i], scope_tree, global_scope_idx);
            i++;
        }
    }
}