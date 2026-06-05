#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "struct.h"

void safe_set_left(ASTTree *tree, int node_index, int left_index) {
    if (node_index < 0 || node_index >= tree->count) {
        printf("\n!!! CRITICAL HEAP HAZARD !!!\n");
        printf("Attempted to set .left on invalid node index: %d\nLine: %d\n Column: %d\n",
            node_index,
            tree->nodes[node_index].line,
            tree->nodes[node_index].column);
        printf("Current tree count: %d, Capacity: %d\n", tree->count, tree->capacity);
        exit(1);
    }
    tree->nodes[node_index].left = left_index;
}

void safe_set_right(ASTTree *tree, int node_index, int right_index) {
    if (node_index < 0 || node_index >= tree->count) {
        printf("\n!!! CRITICAL HEAP HAZARD !!!\n");
        printf("Attempted to set .right on invalid node index: %d\nLine: %d\n Column: %d\n",
            node_index,
            tree->nodes[node_index].line,
            tree->nodes[node_index].column);
        printf("Current tree count: %d, Capacity: %d\n", tree->count, tree->capacity);
        exit(1);
    }
    tree->nodes[node_index].right = right_index;
}

TokenType peek(TokenList *list, int current_idx, int offset) {
    int target = current_idx + offset;

    if (target >= list->count || target < 0) {
        return TOKEN_EOF;
    }
    
    return list->items[target].type;
}

int parse_factor(ASTTree *tree, TokenList *list, int *i, StringPool *pool);
int parse_term(ASTTree *tree, TokenList *list, int *i, StringPool *pool);
int parse_expression(ASTTree *tree, TokenList *list, int *i, StringPool *pool);
int parse_body(ASTTree *tree, TokenList *list, StringPool *pool, int *i);

void print_block(ASTTree *tree, ASTNode *node, StringPool *pool, int node_index, int level) {
    if (node_index == -1 || node == NULL) return;

    // 1. Print the indentation
    for (int i = 0; i < level; i++) printf("  ");

    // 2. Print the current node data
    if (node->type == TOKEN_INT_LIT) {
        printf("|-- [Number]: %d\n", node->data.number_value);
    } else if (node->type == TOKEN_IDENTIFIER) {
        printf("|-- [%s]: %s\n", 
            token_type_to_string(node->type), 
            &pool->data[node->data.string_offset]);
    } else if (node->type == TOKEN_FUNCTION) {
        printf("|-- [FUNCTION]: %s (Returns: %s)\n", 
            &pool->data[node->data.function.name_string_offset],
            token_type_to_string(node->data.function.return_type));
    } else {
        printf("|-- [%s]\n", token_type_to_string(node->type));
    }

    // 3. Traversal logic: Treat blocks and standard expressions distinctly
    if (node->type == TOKEN_BLOCK) {
        int total = node->data.block.statement_count;

        for (int i = 0; i < total; i++) {
            int child_index = node->data.block.statement_indices[i];
            // Render each statement inside the block at the next indentation level
            print_block(tree, &tree->nodes[child_index], pool, child_index, level + 1);
            
            // Keeps your global layout dividers clean
            if (level == 0 && i < total - 1) {
                printf("---------------------------------------\n");
            }
        }
    } else if (node->type == TOKEN_PARAM) {
        int total = node->data.param.param_count;

        for (int i = 0; i < total; i++) {
            int child_index = node->data.block.statement_indices[i];
            // Render each statement inside the block at the next indentation level
            print_block(tree, &tree->nodes[child_index], pool, child_index, level + 1);
            
            // Keeps your global layout dividers clean
            if (level == 0 && i < total - 1) {
                printf("---------------------------------------\n");
            }
        }
        
    } else {
        // Only visit standard left and right binary children if we aren't iterating a block container
        if (node->left != -1)  print_block(tree, &tree->nodes[node->left], pool, node->left, level + 1);
        if (node->right != -1) print_block(tree, &tree->nodes[node->right], pool, node->right, level + 1);
    }
}

void print_ast(ProgramRegistry *registry, ASTTree *tree, StringPool *pool, int level) {
    for (int i = 0; i < registry->function_count; i++) {
        int node_index = registry->global_functions[i];
        print_block(tree, &tree->nodes[registry->global_functions[i]], pool, node_index, level);
    }

    printf("\n--- AST debug ---\n");
    for (int j = 0; j < tree->count; j++) {
        printf("Node %d: Type = %s\n", j, token_type_to_string(tree->nodes[j].type));
    }
}

int create_node(ASTTree *tree, Token token, StringPool *pool) {
    if (tree->count >= tree->capacity) {
        tree->capacity = (tree->capacity == 0) ? 8 : tree->capacity * 2;

        ASTNode *temp = realloc(tree->nodes, tree->capacity * sizeof(ASTNode));
        if (!temp) {
            printf("node failed to reallocate. Out of memory");
            exit(1);
        }
        tree->nodes = temp;
    }

    tree->nodes[tree->count].type = token.type;
    tree->nodes[tree->count].left = -1;
    tree->nodes[tree->count].right = -1;
    tree->nodes[tree->count].line = token.line;
    tree->nodes[tree->count].column = token.column;
    tree->nodes[tree->count].data.number_value = 0;

    switch (tree->nodes[tree->count].type) {
        case TOKEN_INT_LIT: {
            char *raw_string = &pool->data[token.value_offset];
            tree->nodes[tree->count].data.number_value = atoi(raw_string);
            break;
        }

        case TOKEN_CHAR_STRING:
        case TOKEN_INT:
        case TOKEN_CHAR:
        case TOKEN_IF:
        case TOKEN_PRINT:
        case TOKEN_IDENTIFIER:
        case TOKEN_ASSIGN:
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_MULTIPLY:
        case TOKEN_DIVIDE:
        case TOKEN_COMPARE_EQ:
        case TOKEN_NOT_EQ:
        case TOKEN_GREATER_THAN:
        case TOKEN_LESS_THAN:
        case TOKEN_FUNCTION:
        case TOKEN_RETURN:
        case TOKEN_VAR_DECL:
            tree->nodes[tree->count].data.string_offset = token.value_offset;
            break;

        case TOKEN_UNKNOWN:
        case TOKEN_EOF:
        case TOKEN_BLOCK:
        case TOKEN_PARAM:
            break;

        default:
            printf("Unknown type %s. Line %d Column %d",
                token_type_to_string(tree->nodes[tree->count].type),
                tree->nodes[tree->count].line,
                tree->nodes[tree->count].column);
            exit(1);
    }

    return tree->count++;
}

int merge_node(ASTTree *tree, TokenList *list, int *i, int index, char mode, StringPool *pool) {
    int current_node = create_node(tree, list->items[(*i)], pool);
    (*i)++;

    safe_set_left(tree, current_node, index);
    switch (mode) {
        case 't':
            safe_set_right(tree, current_node, parse_term(tree, list, i, pool));
            break;
        
        case 'f':
            safe_set_right(tree, current_node, parse_factor(tree, list, i, pool));
            break;

        case 'e':
            safe_set_right(tree, current_node, parse_expression(tree, list, i, pool));
            break;
        
        default:
            printf("Unknown mode");
            exit(1);
            break;
    }

    return current_node;
}

int parse_factor(ASTTree *tree, TokenList *list, int *i, StringPool *pool) {
    int current_node = create_node(tree, list->items[*i], pool);
    (*i)++;

    return current_node;
}

int parse_term(ASTTree *tree, TokenList *list, int *i, StringPool *pool) {
    int current_node = parse_factor(tree, list, i, pool);

    while (list->items[*i].type == TOKEN_MULTIPLY || list->items[*i].type == TOKEN_DIVIDE) {
        current_node = merge_node(tree, list, i, current_node, 'f', pool);
    }
    
    return current_node;
}

int parse_expression(ASTTree *tree, TokenList *list, int *i, StringPool *pool) {
    int current_node = parse_term(tree, list, i, pool);

    while (list->items[*i].type == TOKEN_PLUS || list->items[*i].type == TOKEN_MINUS) {
        current_node = merge_node(tree, list, i, current_node, 't', pool);
    }

    return current_node;
}

int parse_comparisons(ASTTree *tree, TokenList *list, int *i, StringPool *pool) {
    int current_node = parse_expression(tree, list, i, pool);

    while (list->items[*i].type == TOKEN_COMPARE_EQ || list->items[*i].type == TOKEN_NOT_EQ ||
           list->items[*i].type == TOKEN_GREATER_THAN || list->items[*i].type == TOKEN_LESS_THAN) {
        current_node = merge_node(tree, list, i, current_node, 'e', pool);
    }

    return current_node;
}

void check_semicolon(Token *tokens, int column, int line, int *i) {
    if (tokens[(*i)].type != TOKEN_SEMICOLON) {
        printf("Missing semicolon at index %d line %d Column %d", (*i), line, column);
        exit(1);
    } else {
        (*i)++;
    }
}

int parse_block(TokenList *list, StringPool *pool, int *i, ASTTree *tree) {
    int current_node = -1;
    switch (list->items[(*i)].type) {
        case TOKEN_IDENTIFIER: {
            Token ident_token = list->items[(*i)];
            (*i)++;

            if (list->items[(*i)].type != TOKEN_ASSIGN) {
                printf("Missing equals at index %d line %d Column %d",
                    (*i),
                    list->items[(*i)].line,
                    list->items[(*i)].column);
                exit(1);
            }
            current_node = create_node(tree, list->items[(*i)], pool);
            (*i)++;

            safe_set_left(tree, current_node, create_node(tree, ident_token, pool));
            safe_set_right(tree, current_node, parse_expression(tree, list, i, pool));

            check_semicolon(list->items, list->items[(*i)].column, list->items[(*i)].line, i);

            break;
        }

        case TOKEN_INT:
        case TOKEN_CHAR: {
            Token type_token = list->items[(*i)];
            (*i)++;

            Token var_decl_token = { .type = TOKEN_VAR_DECL, .value_offset = 0 };
            current_node = create_node(tree, var_decl_token, pool);

            tree->nodes[current_node].data.variable_type = type_token.type;

            Token ident_token = list->items[(*i)];
            (*i)++;
            
            if (list->items[(*i)].type != TOKEN_ASSIGN) {
                safe_set_left(tree, current_node, create_node(tree, ident_token, pool));
            } else {
                int assign_node = create_node(tree, list->items[(*i)], pool);
                (*i)++;

                safe_set_left(tree, assign_node, create_node(tree, ident_token, pool));
                safe_set_right(tree, assign_node, parse_expression(tree, list, i, pool));

                safe_set_right(tree, current_node, assign_node);
            }

            check_semicolon(list->items, list->items[(*i)].column, list->items[(*i)].line, i);

            break;
        }

        case TOKEN_PRINT: {
            current_node = create_node(tree, list->items[(*i)], pool);
            (*i)++;

            safe_set_right(tree, current_node, parse_expression(tree, list, i, pool));

            check_semicolon(list->items, list->items[(*i)].column, list->items[(*i)].line, i);

            break;
        }

        case TOKEN_IF:
            current_node = create_node(tree, list->items[(*i)], pool);
            (*i)++;

            if (list->items[(*i)].type != TOKEN_LPAREN) {
                printf("Missing parentheses at index %d line %d column %d",
                    (*i),
                    list->items[(*i)].line,
                    list->items[(*i)].column);
                exit(1);
            }
            (*i)++;

            safe_set_left(tree, current_node, parse_comparisons(tree, list, i, pool));

            if (list->items[(*i)].type != TOKEN_RPAREN) {
                printf("Missing parentheses at index %d line %d column %d",
                    (*i),
                    list->items[(*i)].line,
                    list->items[(*i)].column);
                exit(1);
            }
            (*i)++;

            if (list->items[(*i)].type != TOKEN_LBRACES) {
                printf("Missing braces at index %d line %d column %d",
                    (*i),
                    list->items[(*i)].line,
                    list->items[(*i)].column);
                exit(1);
            }
            (*i)++;
            
            int body_block_index = parse_body(tree, list, pool, i);
            safe_set_right(tree, current_node, body_block_index);
            
            if (list->items[(*i)].type != TOKEN_RBRACES) {
                printf("Missing braces at index %d line %d column %d",
                    (*i),
                    list->items[(*i)].line,
                    list->items[(*i)].column);
                exit(1);
            }
            (*i)++;

            break;
            
        case TOKEN_RETURN:
            current_node = create_node(tree, list->items[(*i)], pool);
            (*i)++;

            if (list->items[(*i)].type != TOKEN_SEMICOLON) {
                safe_set_right(tree, current_node, parse_expression(tree, list, i, pool));
            }

            check_semicolon(list->items, list->items[(*i)].column, list->items[(*i)].line, i);

            break;

        default:
            printf("Unknown token index %d at line %d column %d",
                (*i),
                list->items[(*i)].line,
                list->items[(*i)].column);
            exit(1);
            break;
    }

    return current_node;
}

int parse_body(ASTTree *tree, TokenList *list, StringPool *pool, int *i) {
    Token block_token = { .type = TOKEN_BLOCK, .value_offset = 0 };
    int block_node_index = create_node(tree, block_token, pool);

    int capacity = 8;
    int count = 0;
    int *statements = malloc(capacity * sizeof(int));
    if (!statements) { printf("Statement failed to allocate. Out of memory\n"); exit(1); }
    while (*i < list->count && list->items[(*i)].type != TOKEN_RBRACES) {
        if (count >= capacity) {
            capacity *= 2;
            
            int *temp = realloc(statements, capacity * sizeof(int));
            if (!temp) { printf("Statement failed to reallocate. Out of memory\n"); exit(1); }
            statements = temp;
        }
        
        statements[count] = parse_block(list, pool, i, tree);
        count++;
    }

    if (count > 0) {
        statements = realloc(statements, count *sizeof(int));
    } else {
        free(statements);
        statements = NULL;
    }

    tree->nodes[block_node_index].data.block.statement_indices = statements;
    tree->nodes[block_node_index].data.block.statement_count = count;
    
    return block_node_index;
}

int parse_parameter(ASTTree *tree, TokenList *list, StringPool *pool, int *i) {
    int param_index;
    switch (list->items[(*i)].type) {
        case TOKEN_INT:
        case TOKEN_CHAR:
            param_index = create_node(tree, list->items[(*i)], pool);
            break;

        default:
            (*i)++;
            return -1;
            break;
    }
    (*i)++;
    
    if (list->items[(*i)].type != TOKEN_IDENTIFIER) {
        printf("Expected identifier at index %d line %d column %d",
            (*i),
            list->items[(*i)].line,
            list->items[(*i)].column);
        exit(1);
    }
    safe_set_left(tree, param_index, create_node(tree, list->items[(*i)], pool));
    (*i)++;

    if (list->items[(*i)].type == TOKEN_ASSIGN) {
        (*i)++;
        
        int expr_node = parse_expression(tree, list, i, pool);
        if (expr_node == -1) {
            printf("Expected expression after '=' at index %d line %d column %d\n",
                (*i),
                list->items[*i].line,
                list->items[*i].column);
            exit(1);
        }
        safe_set_right(tree, param_index, expr_node);
    }
    
    return param_index;
}

int parse_parameter_list(ASTTree *tree, TokenList *list, StringPool *pool, int *i) {
    Token param_token = { .type = TOKEN_PARAM, .value_offset = 0 };
    int param_node_index = create_node(tree, param_token, pool);

    int capacity = 8;
    int count = 0;
    int *parameters = malloc(capacity * sizeof(int));
    if (!parameters) { printf("Parameters failed to allocate. Out of memory\n"); exit(1); }
    while (*i < list->count && list->items[(*i)].type != TOKEN_RPAREN) {
        if (count >= capacity) {
            capacity *= 2;
            
            int *temp = realloc(parameters, capacity * sizeof(int));
            if (!temp) { printf("Parameters failed to reallocate. Out of memory\n"); exit(1); }
            parameters = temp;
        }
        
        int param_node_idx =  parse_parameter(tree, list, pool, i);
        if (param_node_idx == -1) {
            if (count > 0) {
                printf("Expected parameter after comma at index %d line %d column %d\n",
                    (*i),
                    list->items[*i].line,
                    list->items[*i].column);
                exit(1);
            }
            break;
        }
        
        parameters[count++] = param_node_idx;
        
        if (list->items[(*i)].type == TOKEN_COMMA) {
            (*i)++;

            if (list->items[*i].type == TOKEN_RPAREN) {
                printf("Trailing comma not allowed in parameter list at index %d line %d column %d\n",
                    (*i),
                    list->items[*i].line,
                    list->items[*i].column);
                exit(1);
            }
        } else if (list->items[(*i)].type != TOKEN_RPAREN) {
            printf("Expected ',' or ')' at index %d line %d column %d\n",
                (*i),
                list->items[*i].line,
                list->items[*i].column);
            exit(1);
        }
    }

    if (count > 0) {
        parameters = realloc(parameters, count *sizeof(int));
    } else {
        free(parameters);
        parameters = NULL;
    }

    tree->nodes[param_node_index].data.param.param_indices = parameters;
    tree->nodes[param_node_index].data.param.param_count = count;
    
    return param_node_index;
}

int parse_program(ProgramRegistry *registry, ASTTree *tree, TokenList *list, StringPool *pool, int *i) {
    TokenType ret_type = list->items[(*i)].type;
    switch (ret_type) {
        case TOKEN_INT:
        case TOKEN_CHAR:
        case TOKEN_VOID:
            break;
        
        default:
            printf("Missing function return type at index %d line %d column %d",
                (*i),
                list->items[(*i)].line,
                list->items[*i].column);
            exit(1);
    }
    (*i)++;

    if (list->items[(*i)].type != TOKEN_IDENTIFIER) {
        printf("Missing function name identifier at index %d line %d column %d",
            (*i),
            list->items[(*i)].line,
            list->items[*i].column);
        exit(1);
    }

    Token func_token = { .type = TOKEN_FUNCTION, .value_offset = list->items[(*i)].value_offset };
    int func_node_index = create_node(tree, func_token, pool);

    tree->nodes[func_node_index].data.function.return_type = ret_type;
    tree->nodes[func_node_index].data.function.name_string_offset = list->items[(*i)].value_offset;

    if (strcmp(&pool->data[list->items[(*i)].value_offset], "main") == 0) {
        registry->main_function_node_index = func_node_index;
    }
    (*i)++;

    if (list->items[(*i)].type != TOKEN_LPAREN) {
        printf("Missing parenthesis at index %d line %d column %d",
            (*i),
            list->items[(*i)].line,
            list->items[*i].column);
        exit(1);
    }
    (*i)++;
    
    // int param_list_idx = parse_parameter_list(tree, list, pool, i);
    // tree->nodes[func_node_index].data.param.param_indices 
    safe_set_left(tree, func_node_index, parse_parameter_list(tree, list, pool, i));
    (*i)++;

    if (list->items[(*i)].type != TOKEN_LBRACES) {
        printf("Missing braces at index %d line %d column %d",
            (*i),
            list->items[(*i)].line,
            list->items[*i].column);
        exit(1);
    }
    (*i)++;

    int body_block_index = parse_body(tree, list, pool, i);
    safe_set_right(tree, func_node_index, body_block_index);

    if (list->items[(*i)].type != TOKEN_RBRACES) {
        printf("Missing braces at index %d line %d column %d",
            (*i),
            list->items[(*i)].line,
            list->items[*i].column);
        exit(1);
    }
    (*i)++;

    return func_node_index;
}

void ensure_registry_capacity(ProgramRegistry *registry) {
    if (registry->function_count >= registry->function_capacity) {
        registry->function_capacity = (registry->function_capacity == 0) ? 8 : registry->function_capacity * 2;

        int *temp = realloc(registry->global_functions, registry->function_capacity * sizeof(int));
        if (!temp) {
            printf("func registry failed to reallocate. Out of memory");
            exit(1);
        }
        registry->global_functions = temp;
    }
    
    if (registry->global_var_count >= registry->global_var_capacity) {
        registry->global_var_capacity = (registry->global_var_capacity == 0) ? 8 : registry->global_var_capacity * 2;

        int *temp = realloc(registry->global_variables, registry->global_var_capacity * sizeof(int));
        if (!temp) {
            printf("global var registry failed to reallocate. Out of memory");
            exit(1);
        }
        registry->global_variables = temp;
    }
}

void parsing(TokenList *list, ProgramRegistry *registry, StringPool *pool, ASTTree *tree, int *program_root) {
    if (list == NULL || list->items == NULL) {
        printf("Error");
        exit(1);
    }

    int i = 0, modifiable = 0;
    while (i < list->count && list->items[i].type != TOKEN_EOF) {
        ensure_registry_capacity(registry);
        if (peek(list, i, 2) == TOKEN_ASSIGN || peek(list, i, 2) == TOKEN_SEMICOLON) {
            registry->global_variables[registry->global_var_count++] = parse_block(list, pool, &i, tree);
        } else if (peek(list, i, 2) == TOKEN_LPAREN) {
            registry->global_functions[registry->function_count++] = parse_program(registry, tree, list, pool, &i);
        }
    }
}
