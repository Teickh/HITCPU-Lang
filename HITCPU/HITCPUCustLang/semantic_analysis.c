#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "struct.h"

int create_new_scope(ScopeStack *scope_stack) {
    if (scope_stack->stack + 1 >= scope_stack->scope_capacity) {
        scope_stack->scope_capacity *= 2;

        SymbolTable *temp = realloc(scope_stack->scopes, scope_stack->scope_capacity * sizeof(SymbolTable));
        if (!temp) {
            printf("Scope stack scopes failed to reallocate. Out of memory");
            exit(1);
        }
        scope_stack->scopes = temp;
    }
    
    scope_stack->stack++;
    scope_stack->scopes[scope_stack->stack].symbol_count = 0;
    scope_stack->scopes[scope_stack->stack].symbol_capacity = 8;
    scope_stack->scopes[scope_stack->stack].symbols = malloc(8 * sizeof(Symbol));

    return scope_stack->stack;
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

int lookup_symbol(ScopeStack *scope_stack, int current_scope_idx, int string_offset) {
    int index = current_scope_idx;
    while (index >= 0) {
        SymbolTable current_scope_table = scope_stack->scopes[index];
        for (int i = 0; i < current_scope_table.symbol_count; i++) {
            if (current_scope_table.symbols[i].string_offset == string_offset) {
                return index;
            }
        }
        index--;
    }
    
    return -1;
}

// Deep copy helper function to preserve historical scopes safely
void archive_scope_history(SymbolLists *symbol_lists, SymbolTable *source_table) {
    if (symbol_lists->count >= symbol_lists->capacity) {
        symbol_lists->capacity = symbol_lists->capacity == 0 ? 4 : symbol_lists->capacity * 2;

        SymbolTable *temp = realloc(symbol_lists->historical_scopes, symbol_lists->capacity * sizeof(SymbolTable));
        if (!temp) {
            printf("Historical scope array failed to reallocate. Out of memory\n");
            exit(1);
        }
        symbol_lists->historical_scopes = temp;
    }

    // Allocate brand-new heap space for historical symbols to avoid cross-contamination
    SymbolTable *dest_table = &symbol_lists->historical_scopes[symbol_lists->count++];
    dest_table->symbol_count = source_table->symbol_count;
    dest_table->symbol_capacity = source_table->symbol_capacity;
    dest_table->symbols = malloc(source_table->symbol_capacity * sizeof(Symbol));
    
    if (dest_table->symbols && source_table->symbols) {
        memcpy(dest_table->symbols, source_table->symbols, source_table->symbol_count * sizeof(Symbol));
    }
}

void analyse_parameters(ASTTree *tree, int param_node_idx, ScopeStack *scope_stack, int current_scope_idx) {
    if (param_node_idx == -1) return;

    ASTNode *param_node = &tree->nodes[param_node_idx];
    if (param_node->type == TOKEN_INT || param_node->type == TOKEN_CHAR) {
        if (param_node->left != -1) {
            ASTNode *ident_node = &tree->nodes[param_node->left];

            printf("Parameter: Registering input %s variable with string_offset %d. Line %d Column %d\n",
                token_type_to_string(ident_node->type),
                ident_node->data.string_offset,
                param_node->line,
                param_node->column);

            add_symbol(&scope_stack->scopes[current_scope_idx], SYMBOL_LOCAL, ident_node->data.string_offset, ident_node->type);
        }
    }
}

void analyse_block(ASTTree *tree, int current_node_idx, SymbolLists *symbol_lists, ScopeStack *scope_stack, int current_scope_idx, int is_function_block, SymbolType symbol_type) {
    if (current_node_idx == -1) return;

    ASTNode *current_node = &tree->nodes[current_node_idx];
    int symbol_exists = 0;
    switch (current_node->type) {
        case TOKEN_FUNCTION: {
            add_symbol(&scope_stack->scopes[current_scope_idx], SYMBOL_FUNCTION, tree->nodes[current_node_idx].data.string_offset, tree->nodes[current_node_idx].data.variable_type);
            current_scope_idx = create_new_scope(scope_stack);

            int param_list_idx = current_node->left;
            if (param_list_idx != -1) {
                ASTNode *param_node = &tree->nodes[param_list_idx];

                if (param_node->type == TOKEN_PARAM) {
                    for (int i = 0; i < param_node->data.param.param_count; i++) {
                        analyse_parameters(tree, param_node->data.param.param_indices[i], scope_stack, current_scope_idx);
                    }
                }
            }
            
            if (current_node->right != -1) {
                analyse_block(tree, current_node->right, symbol_lists, scope_stack, current_scope_idx, 1, symbol_type);
            }
            
            archive_scope_history(symbol_lists, &scope_stack->scopes[current_scope_idx]);
            free(scope_stack->scopes[current_scope_idx].symbols);
            scope_stack->stack--;
            break;
        }
        
        case TOKEN_BLOCK: {
            int created_new = 0;
            if (is_function_block != 1) {
                current_scope_idx = create_new_scope(scope_stack);
                created_new = 1;
            }

            int total = current_node->data.block.statement_count;
            int *stmts = current_node->data.block.statement_indices;
            for (int i = 0; i < total; i++) {
                analyse_block(tree, stmts[i], symbol_lists, scope_stack, current_scope_idx, 0, symbol_type);
            }
            
            if (created_new) {
                archive_scope_history(symbol_lists, &scope_stack->scopes[current_scope_idx]);
                free(scope_stack->scopes[current_scope_idx].symbols);
                scope_stack->stack--;
            }
            break;
        }

        case TOKEN_IDENTIFIER: {
            printf("Variable Usage: Verifying variable %d exists to read its value. Line %d Column %d\n", 
                    current_node->data.string_offset,
                    current_node->line,
                    current_node->column);

            symbol_exists = lookup_symbol(scope_stack, current_scope_idx, current_node->data.string_offset);
            
            if (symbol_exists == -1) {
                printf("Error: variable hasn't been initialised. Line %d Column %d",
                    current_node->line,
                    current_node->column);
                exit(1);
            }
            break;
        }

        case TOKEN_VAR_DECL: {
            int assign_node_index = current_node->right;

            if (assign_node_index != -1) {
                ASTNode *assign_node = &tree->nodes[assign_node_index];
                ASTNode *identification_node = &tree->nodes[assign_node->left];
                
                symbol_exists = lookup_symbol(scope_stack, current_scope_idx, identification_node->data.string_offset);
                if (symbol_exists != -1) {
                    printf("Error: variable has already been initialised. Line %d Column %d\n",
                        tree->nodes[current_node->left].line,
                        tree->nodes[current_node->left].column);
                    exit(1);
                }
            
                printf("Declaration: Registering new %s variable with string_offset %d. Line %d Column %d\n", 
                    token_type_to_string(identification_node->data.variable_type),
                    identification_node->data.string_offset,
                    identification_node->line,
                    identification_node->column);

                symbol_type = SYMBOL_LOCAL;
                add_symbol(&scope_stack->scopes[current_scope_idx], symbol_type, identification_node->data.string_offset, identification_node->data.variable_type);

                if (assign_node->right != -1) {
                    analyse_block(tree, assign_node->right, symbol_lists, scope_stack, current_scope_idx, 0, symbol_type);
                }
            } else {
                symbol_exists = lookup_symbol(scope_stack, current_scope_idx, tree->nodes[current_node->left].data.string_offset);
                if (symbol_exists != -1) {
                    printf("Error: variable has already been initialised. Line %d Column %d\n",
                        tree->nodes[current_node->left].line,
                        tree->nodes[current_node->left].column);
                    exit(1);
                }
                
                printf("Declaration: Registering new %s variable with string_offset %d. Line %d Column %d\n",
                    token_type_to_string(tree->nodes[current_node->left].data.variable_type),
                    tree->nodes[current_node->left].data.string_offset,
                    tree->nodes[current_node->left].line,
                    tree->nodes[current_node->left].column);

                add_symbol(&scope_stack->scopes[current_scope_idx], symbol_type, tree->nodes[current_node->left].data.string_offset, current_node->data.variable_type);
            }
            break;
        }

        case TOKEN_ASSIGN: {
            int identification_node_index = current_node->left;
            ASTNode *identification_node = &tree->nodes[identification_node_index];

            printf("Assignment: Checking if variable %d exists in symbol table... Line %d Column %d\n",
                    identification_node->data.string_offset,
                    identification_node->line,
                    identification_node->column);

            symbol_exists = lookup_symbol(scope_stack, scope_stack->stack, identification_node->data.string_offset);

            if (symbol_exists == -1) {
                printf("Error: variable hasn't been initialised. Line %d Column %d",
                    tree->nodes[current_node->left].line,
                    tree->nodes[current_node->left].column);
                exit(1);
            }

            analyse_block(tree, current_node->right, symbol_lists, scope_stack, current_scope_idx, 0, symbol_type);
            break;
        }
        
        default:
            if (current_node->left != -1)  analyse_block(tree, current_node->left, symbol_lists, scope_stack, current_scope_idx, 0, symbol_type);
            if (current_node->right != -1) analyse_block(tree, current_node->right, symbol_lists, scope_stack, current_scope_idx, 0, symbol_type);
            break;
    }
}

void analyse(FileRegistry *registry, ASTTree *tree, ScopeStack *scope_stack, SymbolLists *symbol_lists) {
    int global_scope_idx = create_new_scope(scope_stack);

    for (int i = 0; i < registry->global_var_count; i++) {
        analyse_block(tree, registry->global_variables[i], symbol_lists, scope_stack, global_scope_idx, 0, SYMBOL_GLOBAL);
    }

    for (int i = 0; i < registry->function_count; i++) {
        analyse_block(tree, registry->global_functions[i], symbol_lists, scope_stack, global_scope_idx, 1, SYMBOL_FUNCTION);
    }
}