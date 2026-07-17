#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "struct.h"

void log_historical_scopes_html(FILE *html_file, SymbolLists *symbol_lists, StringPool *pool) {
    if (!html_file || !symbol_lists) return;

    // Add a visual separator and a new section heading that matches the Bootstrap aesthetic
    fprintf(html_file, "<hr style=\"margin-top: 40px; border: 0; border-top: 2px dashed #dee2e6;\">\n");
    fprintf(html_file, "<h2 style=\"color: #007bff; margin-top: 30px;\">Archived Historical Scopes</h2>\n");
    fprintf(html_file, "<p>Below is a snapshot of all scopes preserved in history after exiting their blocks:</p>\n");

    if (symbol_lists->count == 0) {
        fprintf(html_file, "<p style=\"font-style: italic; color: #6c757d;\">No historical scopes were archived.</p>\n");
        return;
    }

    // Loop through every archived scope table
    for (int s = 0; s < symbol_lists->count; s++) {
        SymbolTable *table = &symbol_lists->historical_scopes[s];
        
        // Cleaned up card containers for light mode
        fprintf(html_file, "<div style=\"margin-bottom: 25px; border: 1px solid #dee2e6; border-radius: 6px; background-color: #fff; overflow: hidden;\">\n");
        fprintf(html_file, "  <div style=\"background-color: #f8f9fa; padding: 10px 15px; border-bottom: 1px solid #dee2e6; font-weight: bold; color: #333;\">\n");
        fprintf(html_file, "    Scope Archive Index [%d] <span style=\"font-weight: normal; font-size: 0.9em; color: #6c757d; margin-left: 15px;\">(Symbols: %d / Capacity: %d)</span>\n", 
                s, table->symbol_count, table->symbol_capacity);
        fprintf(html_file, "    <span style=\"font-weight: normal; font-size: 0.9em; color: #6c757d; margin-left: 15px;\">Parent Scope Index: [%d]</span>\n", 
                table->parent_scope);
        fprintf(html_file, "  </div>\n");

        if (table->symbol_count == 0) {
            fprintf(html_file, "  <div style=\"padding: 15px; color: #6c757d; font-style: italic;\">Empty scope (no variables declared).</div>\n");
            fprintf(html_file, "</div>\n");
            continue;
        }

        // Sub-table rendering cleanly with light mode classes
        fprintf(html_file, "  <table style=\"margin: 0; border-radius: 0 0 6px 6px;\">\n");
        fprintf(html_file, "    <thead>\n");
        fprintf(html_file, "      <tr style=\"background-color: #007bff; color: white;\">\n");
        fprintf(html_file, "        <th style=\"background-color: #6c757d; color: white; width: 10%%;\">No.</th>\n");
        fprintf(html_file, "        <th style=\"background-color: #6c757d; color: white; width: 20%%;\">Symbol Type</th>\n");
        fprintf(html_file, "        <th style=\"background-color: #6c757d; color: white; width: 20%%;\">Data Type (Enum)</th>\n");
        fprintf(html_file, "        <th style=\"background-color: #6c757d; color: white; width: 25%%;\">String Offset</th>\n");
        fprintf(html_file, "        <th style=\"background-color: #6c757d; color: white; width: 25%%;\">String Name</th>\n");
        fprintf(html_file, "      </tr>\n");
        fprintf(html_file, "    </thead>\n");
        fprintf(html_file, "    <tbody>\n");

        for (int i = 0; i < table->symbol_count; i++) {
            Symbol *sym = &table->symbols[i];

            const char *type_str = "UNKNOWN";
            const char *badge_class = "info"; // Fallback to info class from your style settings
            
            switch (sym->type) {
                case SYMBOL_GLOBAL:    type_str = "GLOBAL";    badge_class = "success"; break;
                case SYMBOL_LOCAL:     type_str = "LOCAL";     badge_class = "info"; break;
                case SYMBOL_FUNCTION:  type_str = "FUNCTION";  badge_class = "warning"; break;
                case SYMBOL_PARAM:     type_str = "PARAM";     badge_class = "badge-param"; break;
                case SYMBOL_CONDITION: type_str = "CONDITION"; badge_class = "badge-param"; break;
                case SYMBOL_IF:        type_str = "IF";        badge_class = "badge-param"; break;
            }

            const char *resolved_name = "N/A";
            if (pool && sym->string_offset >= 0) {
                resolved_name = &pool->data[sym->string_offset];
            }

            fprintf(html_file, "      <tr>\n");
            fprintf(html_file, "        <td>%d</td>\n", i);
            fprintf(html_file, "        <td><span class=\"badge %s\">%s</span></td>\n", badge_class, type_str);
            fprintf(html_file, "        <td><code>%d</code></td>\n", sym->data_type); 
            fprintf(html_file, "        <td><code>%d</code></td>\n", sym->string_offset);
            fprintf(html_file, "        <td><strong style=\"color: #495057;\">%s</strong></td>\n", resolved_name);
            fprintf(html_file, "      </tr>\n");
        }

        fprintf(html_file, "    </tbody>\n");
        fprintf(html_file, "  </table>\n");
        fprintf(html_file, "</div>\n");
    }
}

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

void add_symbol(ScopeStack *scope_stack, int current_scope_idx, SymbolType type, int string_offset, TokenType data_type) {
    SymbolTable *table = &scope_stack->scopes[current_scope_idx];
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
    table->symbols[table->symbol_count].param_index = -1;
    table->symbols[table->symbol_count].stack_offset = 0;
    table->symbol_count++;
}

// 1. Call this when entering a block to get a permanent ID
int reserve_archive_slot(SymbolLists *symbol_lists) {
    if (symbol_lists->count >= symbol_lists->capacity) {
        symbol_lists->capacity = symbol_lists->capacity == 0 ? 4 : symbol_lists->capacity * 2;
        SymbolTable *temp = realloc(symbol_lists->historical_scopes, symbol_lists->capacity * sizeof(SymbolTable));
        if (!temp) { printf("Out of memory\n"); exit(1); }
        symbol_lists->historical_scopes = temp;
    }
    
    // Grab the current index and increment count so nobody else takes it
    int reserved_idx = symbol_lists->count++;
    
    // Initialize it to safe defaults so it's not garbage if we inspect it mid-flight
    symbol_lists->historical_scopes[reserved_idx].symbol_count = 0;
    symbol_lists->historical_scopes[reserved_idx].symbols = NULL;
    
    return reserved_idx;
}

// 2. Call this when exiting a block to write the symbols into your reserved ID
void commit_scope_history(SymbolLists *symbol_lists, int reserved_idx, SymbolTable *source_table, int parent_archive_idx) {
    SymbolTable *dest_table = &symbol_lists->historical_scopes[reserved_idx];
    
    dest_table->symbol_count = source_table->symbol_count;
    dest_table->symbol_capacity = source_table->symbol_capacity;
    dest_table->parent_scope = parent_archive_idx;
    dest_table->symbols = malloc(source_table->symbol_capacity * sizeof(Symbol));
    
    if (dest_table->symbols && source_table->symbols) {
        memcpy(dest_table->symbols, source_table->symbols, source_table->symbol_count * sizeof(Symbol));
    }
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

// Helper to safely write HTML logs
void log_html_row(FILE *html_file, const char *action, const char *status_class, const char *details, int line, int col) {
    if (!html_file) return;
    fprintf(html_file, "<tr>\n");
    fprintf(html_file, "  <td><span class=\"badge %s\">%s</span></td>\n", status_class, action);
    fprintf(html_file, "  <td>%s</td>\n", details);
    fprintf(html_file, "  <td>Line %d, Col %d</td>\n", line, col);
    fprintf(html_file, "</tr>\n");
}

void analyse_parameters(ASTTree *tree, int param_node_idx, ScopeStack *scope_stack, int current_scope_idx, FILE *html_file) {
    if (param_node_idx == -1) return;

    ASTNode *param_node = &tree->nodes[param_node_idx];
    if (param_node->type == TOKEN_INT || param_node->type == TOKEN_CHAR) {
        if (param_node->left != -1) {
            ASTNode *ident_node = &tree->nodes[param_node->left];

            char detail_msg[256];
            snprintf(detail_msg, sizeof(detail_msg), "Registering input <strong>%s</strong> variable with string_offset <code>%d</code>.",
                     token_type_to_string(ident_node->type), ident_node->data.string_offset);
            
            log_html_row(html_file, "Parameter", "info", detail_msg, param_node->line, param_node->column);

            add_symbol(scope_stack, current_scope_idx, SYMBOL_PARAM, ident_node->data.string_offset, ident_node->type);
            
            scope_stack->scopes[current_scope_idx].symbols->param_index++;
        }
    }
}

void analyse_block(
    ASTTree *tree,
    int current_node_idx,
    SymbolLists *symbol_lists,
    ScopeStack *scope_stack,
    int current_scope_idx,
    int is_existing_block,
    SymbolType symbol_type,
    FILE *html_file,
    int parent_archive_idx
) {
    if (current_node_idx == -1) return;

    ASTNode *current_node = &tree->nodes[current_node_idx];
    int symbol_exists = 0;
    char detail_msg[512];
    current_node->scope_id = parent_archive_idx;

    switch (current_node->type) {
        case TOKEN_FUNCTION: {
            add_symbol(scope_stack, current_scope_idx, SYMBOL_FUNCTION, tree->nodes[current_node_idx].data.function.name_string_offset, tree->nodes[current_node_idx].data.variable_type);
            current_scope_idx = create_new_scope(scope_stack);
            int my_archive_idx = reserve_archive_slot(symbol_lists);

            int param_list_idx = current_node->left;
            if (param_list_idx != -1) {
                ASTNode *param_node = &tree->nodes[param_list_idx];

                if (param_node->type == TOKEN_PARAM) {
                    for (int i = 0; i < param_node->data.param.param_count; i++) {
                        analyse_parameters(tree, param_node->data.param.param_indices[i], scope_stack, current_scope_idx, html_file);
                    }
                }
            }

            if (current_node->right != -1) {
                analyse_block(tree, current_node->right, symbol_lists, scope_stack, current_scope_idx, 1, symbol_type, html_file, my_archive_idx);
            }
            
            commit_scope_history(symbol_lists, my_archive_idx, &scope_stack->scopes[current_scope_idx], parent_archive_idx);
            current_node->scope_id = my_archive_idx;
            free(scope_stack->scopes[current_scope_idx].symbols);
            scope_stack->stack--;
            break;
        }

        case TOKEN_IF: {
            add_symbol(scope_stack, current_scope_idx, SYMBOL_IF, tree->nodes[current_node_idx].data.function.name_string_offset, tree->nodes[current_node_idx].data.variable_type);
            current_scope_idx = create_new_scope(scope_stack);
            int my_archive_idx = reserve_archive_slot(symbol_lists);

            int condition_idx = current_node->data.if_statement.condition_idx;
            if (condition_idx != -1) {
                analyse_block(tree, condition_idx, symbol_lists, scope_stack, current_scope_idx, 1, SYMBOL_CONDITION, html_file, my_archive_idx);
            }

            int true_block_idx = current_node->data.if_statement.true_block_idx;
            if (true_block_idx != -1) {
                analyse_block(tree, true_block_idx, symbol_lists, scope_stack, current_scope_idx, 1, symbol_type, html_file, my_archive_idx);
            }

            int false_block_idx = current_node->data.if_statement.false_block_idx;
            if (false_block_idx != -1) {
                analyse_block(tree, false_block_idx, symbol_lists, scope_stack, current_scope_idx, 1, symbol_type, html_file, my_archive_idx);
            }

            commit_scope_history(symbol_lists, my_archive_idx, &scope_stack->scopes[current_scope_idx], parent_archive_idx);
            current_node->scope_id = my_archive_idx;
            free(scope_stack->scopes[current_scope_idx].symbols);
            scope_stack->stack--;
            break;
        }
        
        case TOKEN_BLOCK: {
            int created_new = 0;
            int my_archive_idx = parent_archive_idx;
            if (is_existing_block != 1) {
                current_scope_idx = create_new_scope(scope_stack);
                created_new = 1;
                my_archive_idx = reserve_archive_slot(symbol_lists);
            }

            int total = current_node->data.block.statement_count;
            int *stmts = current_node->data.block.statement_indices;
            for (int i = 0; i < total; i++) {
                analyse_block(tree, stmts[i], symbol_lists, scope_stack, current_scope_idx, 0, symbol_type, html_file, my_archive_idx);
            }
            
            if (created_new) {
                commit_scope_history(symbol_lists, my_archive_idx, &scope_stack->scopes[current_scope_idx], parent_archive_idx);
                current_node->scope_id = my_archive_idx;
                // free(scope_stack->scopes[current_scope_idx].symbols);
                // scope_stack->stack--;
            }
            break;
        }

        case TOKEN_IDENTIFIER: {
            current_node->scope_id = parent_archive_idx;

            snprintf(detail_msg, sizeof(detail_msg), "Verifying variable <code>%d</code> exists to read value.", current_node->data.string_offset);
            log_html_row(html_file, "Usage Check", "warning", detail_msg, current_node->line, current_node->column);

            symbol_exists = lookup_symbol(scope_stack, current_scope_idx, current_node->data.string_offset);
            
            if (symbol_exists == -1) {
                snprintf(detail_msg, sizeof(detail_msg), "<strong>Fatal Error:</strong> Variable <code>%d</code> has not been initialized!", current_node->data.string_offset);
                log_html_row(html_file, "Error", "danger", detail_msg, current_node->line, current_node->column);
                
                // Close tags cleanly before exiting
                fprintf(html_file, "</table></div></body></html>\n");
                fclose(html_file);
                exit(1);
            }
            break;
        }

        case TOKEN_VAR_DECL: {
            current_node->scope_id = parent_archive_idx;

            int assign_node_index = current_node->right;

            if (assign_node_index != -1) {
                ASTNode *assign_node = &tree->nodes[assign_node_index];
                ASTNode *identification_node = &tree->nodes[assign_node->left];
                
                symbol_exists = lookup_symbol(scope_stack, current_scope_idx, identification_node->data.string_offset);
                if (symbol_exists != -1 && symbol_exists == current_scope_idx) {
                    snprintf(detail_msg, sizeof(detail_msg), "<strong>Fatal Error:</strong> Variable <code>%d</code> has already been declared in this scope.", identification_node->data.string_offset);
                    log_html_row(html_file, "Error", "danger", detail_msg, identification_node->line, identification_node->column);
                    fprintf(html_file, "</table></div></body></html>\n");
                    fclose(html_file);
                    exit(1);
                }
            
                snprintf(detail_msg, sizeof(detail_msg), "Registering new <strong>%s</strong> variable with string_offset <code>%d</code>.", 
                         token_type_to_string(current_node->data.variable_type), identification_node->data.string_offset);
                log_html_row(html_file, "Declaration", "success", detail_msg, identification_node->line, identification_node->column);

                symbol_type = (current_scope_idx == 0) ? SYMBOL_GLOBAL : SYMBOL_LOCAL;
                add_symbol(scope_stack, current_scope_idx, symbol_type, identification_node->data.string_offset, current_node->data.variable_type);

                if (assign_node->right != -1) {
                    analyse_block(tree, assign_node->right, symbol_lists, scope_stack, current_scope_idx, 0, symbol_type, html_file, parent_archive_idx);
                }
            } else {
                ASTNode *target_node = &tree->nodes[current_node->left];
                symbol_exists = lookup_symbol(scope_stack, current_scope_idx, target_node->data.string_offset);
                if (symbol_exists != -1) {
                    snprintf(detail_msg, sizeof(detail_msg), "<strong>Fatal Error:</strong> Variable <code>%d</code> has already been initialized.", target_node->data.string_offset);
                    log_html_row(html_file, "Error", "danger", detail_msg, target_node->line, target_node->column);
                    fprintf(html_file, "</table></div></body></html>\n");
                    fclose(html_file);
                    exit(1);
                }
                
                snprintf(detail_msg, sizeof(detail_msg), "Registering new <strong>%s</strong> variable with string_offset <code>%d</code>.",
                         token_type_to_string(current_node->data.variable_type), target_node->data.string_offset);
                log_html_row(html_file, "Declaration", "success", detail_msg, target_node->line, target_node->column);

                symbol_type = (current_scope_idx == 0) ? SYMBOL_GLOBAL : SYMBOL_LOCAL;
                add_symbol(scope_stack, current_scope_idx, symbol_type, target_node->data.string_offset, current_node->data.variable_type);
            }
            break;
        }

        case TOKEN_ASSIGN: {
            current_node->scope_id = parent_archive_idx;
            
            int identification_node_index = current_node->left;
            ASTNode *identification_node = &tree->nodes[identification_node_index];

            snprintf(detail_msg, sizeof(detail_msg), "Checking if variable <code>%d</code> exists in symbol table for assignment...", identification_node->data.string_offset);
            log_html_row(html_file, "Assignment", "info", detail_msg, identification_node->line, identification_node->column);

            symbol_exists = lookup_symbol(scope_stack, scope_stack->stack, identification_node->data.string_offset);

            if (symbol_exists == -1) {
                snprintf(detail_msg, sizeof(detail_msg), "<strong>Fatal Error:</strong> Variable <code>%d</code> has not been initialized.", identification_node->data.string_offset);
                log_html_row(html_file, "Error", "danger", detail_msg, tree->nodes[current_node->left].line, tree->nodes[current_node->left].column);
                fprintf(html_file, "</table></div></body></html>\n");
                fclose(html_file);
                exit(1);
            }

            analyse_block(tree, current_node->right, symbol_lists, scope_stack, current_scope_idx, 0, symbol_type, html_file, parent_archive_idx);
            break;
        }

        case TOKEN_COMPARE_EQ:
        case TOKEN_NOT_EQ:
        case TOKEN_GREATER_THAN:
        case TOKEN_GREATER_THAN_OR_EQ:
        case TOKEN_LESS_THAN:
        case TOKEN_LESS_THAN_OR_EQ:
        case TOKEN_AND:
        case TOKEN_OR:
            analyse_block(tree, current_node->left, symbol_lists, scope_stack, current_scope_idx, 0, symbol_type, html_file, parent_archive_idx);
            analyse_block(tree, current_node->right, symbol_lists, scope_stack, current_scope_idx, 0, symbol_type, html_file, parent_archive_idx);
            break;
        
        default:
            if (current_node->left != -1)  analyse_block(tree, current_node->left, symbol_lists, scope_stack, current_scope_idx, 0, symbol_type, html_file, parent_archive_idx);
            if (current_node->right != -1) analyse_block(tree, current_node->right, symbol_lists, scope_stack, current_scope_idx, 0, symbol_type, html_file, parent_archive_idx);
            break;
    }
}

void analyse(FileRegistry *registry, ASTTree *tree, ScopeStack *scope_stack, SymbolLists *symbol_lists, const char *output_filename, StringPool *pool) {
    FILE *html_file = fopen(output_filename, "w");
    if (!html_file) {
        printf("Failed to create compilation report file: %s\n", output_filename);
        return;
    }

    // Write CSS Styles & HTML Blueprint Header
    fprintf(html_file, "<!DOCTYPE html>\n<html>\n<head>\n<title>Static Analysis Log</title>\n");
    fprintf(html_file, "<style>\n");
    fprintf(html_file, "  body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; background-color: #f8f9fa; color: #333; margin: 30px; }\n");
    fprintf(html_file, "  .container { max-width: 1000px; margin: auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }\n");
    fprintf(html_file, "  h2 { border-bottom: 2px solid #007bff; padding-bottom: 10px; color: #007bff; }\n");
    fprintf(html_file, "  table { width: 100%%; border-collapse: collapse; margin-top: 20px; }\n");
    fprintf(html_file, "  th, td { padding: 12px; text-align: left; border-bottom: 1px solid #dee2e6; }\n");
    fprintf(html_file, "  th { background-color: #007bff; color: white; }\n");
    fprintf(html_file, "  tr:hover { background-color: #f1f3f5; }\n");
    fprintf(html_file, "  code { background-color: #e9ecef; padding: 2px 6px; border-radius: 4px; font-family: monospace; font-size: 0.95em; }\n");
    fprintf(html_file, "  .badge { padding: 4px 8px; border-radius: 4px; font-weight: bold; font-size: 0.85em; text-transform: uppercase; color: white; }\n");
    fprintf(html_file, "  .info { background-color: #17a2b8; }\n");
    fprintf(html_file, "  .success { background-color: #28a745; }\n");
    fprintf(html_file, "  .warning { background-color: #ffc107; color: #212529; }\n");
    fprintf(html_file, "  .danger { background-color: #dc3545; }\n");
    fprintf(html_file, "  .badge-param { background-color: #20c997; }\n");
    fprintf(html_file, "</style>\n</head>\n<body>\n<div class=\"container\">\n<h2>Compiler Analysis Report</h2>\n");
    fprintf(html_file, "<table>\n<thead><tr><th>Action</th><th>Description</th><th>Location</th></tr></thead>\n<tbody>\n");

    int global_scope_idx = create_new_scope(scope_stack);
    int global_archive_idx = reserve_archive_slot(symbol_lists);

    for (int i = 0; i < registry->global_var_count; i++) {
        analyse_block(tree, registry->global_variables[i], symbol_lists, scope_stack, global_scope_idx, 0, SYMBOL_GLOBAL, html_file, global_archive_idx);
    }

    for (int i = 0; i < registry->function_count; i++) {
        analyse_block(tree, registry->global_functions[i], symbol_lists, scope_stack, global_scope_idx, 1, SYMBOL_FUNCTION, html_file, global_archive_idx);
    }

    commit_scope_history(symbol_lists, global_archive_idx, &scope_stack->scopes[global_scope_idx], -1);
    tree->nodes[0].scope_id = global_scope_idx;
    free(scope_stack->scopes[global_scope_idx].symbols);
    scope_stack->stack--;

    fprintf(html_file, "</tbody>\n</table>\n");

    log_historical_scopes_html(html_file, symbol_lists, pool);

    // HTML Footer Closure
    fprintf(html_file, "</div>\n</body>\n</html>\n");
    fclose(html_file);
    printf("Static Analysis complete. Log file written to: %s\n", output_filename);
}