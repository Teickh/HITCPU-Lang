#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "struct.h"

// Helper to handle HTML escaping for safety (if variable names contain <, >, &, etc.)
void fprintf_escaped(FILE *out, const char *str) {
    while (*str) {
        switch (*str) {
            case '<': fprintf(out, "&lt;"); break;
            case '>': fprintf(out, "&gt;"); break;
            case '&': fprintf(out, "&amp;"); break;
            default:  fputc(*str, out); break;
        }
        str++;
    }
}

void print_block_html(FILE *out, ASTTree *tree, ASTNode *node, StringPool *pool, int node_index) {
    if (node_index == -1 || node == NULL) return;

    // Determine if this node should be collapsible (has children)
    int has_children = 0;
    if (node->type == TOKEN_BLOCK && node->data.block.statement_count > 0) has_children = 1;
    else if (node->type == TOKEN_PARAM && node->data.param.param_count > 0) has_children = 1;
    else if (node->type == TOKEN_ARG_LIST && node->data.param.param_count > 0) has_children = 1;
    else if (node->type == TOKEN_FUNCTION_CALL && (node->left != -1 || node->right != -1)) has_children = 1;
    else if (node->type == TOKEN_IF) has_children = 1;
    // --- ADDED: Explicitly mark logical operators as structurally collapsible ---
    else if ((node->type == TOKEN_AND || node->type == TOKEN_OR) && (node->left != -1 || node->right != -1)) has_children = 1;
    else if (node->type != TOKEN_BLOCK && node->type != TOKEN_PARAM && node->type != TOKEN_ARG_LIST && node->type != TOKEN_FUNCTION_CALL && (node->left != -1 || node->right != -1)) has_children = 1;

    // Open HTML list item and layout container
    fprintf(out, "<li>\n");
    if (has_children) {
        fprintf(out, "  <details open>\n  <summary>");
    } else {
        fprintf(out, "  <span class='leaf-content'>");
    }

    // Render the node contents based on type
    fprintf(out, "<span class='node-tag node-%s'>%s</span>", 
            token_type_to_string(node->type), token_type_to_string(node->type));

    // 🟢 ADDED: Render scope ID badge using your existing CSS class
    // Adjust the condition (e.g., node->scope_id >= 0) based on how you initialize default scope IDs
    if (node->scope_id != -1) {
        fprintf(out, "<span class='node-scope'>Scope ID: %d</span>", node->scope_id);
    }

    // Render unique properties of specific nodes
    if (node->type == TOKEN_INT_LIT) {
        fprintf(out, " <span class='node-val'>Value: %d</span>", node->data.number_value);
    } else if (node->type == TOKEN_IDENTIFIER) {
        fprintf(out, " <span class='node-val'>");
        fprintf_escaped(out, &pool->data[node->data.string_offset]);
        fprintf(out, "</span>");
    } else if (node->type == TOKEN_FUNCTION) {
        fprintf(out, " <span class='node-val'>Name: ");
        fprintf_escaped(out, &pool->data[node->data.function.name_string_offset]);
        fprintf(out, " (Returns: %s)</span>", token_type_to_string(node->data.function.return_type));
    } else if (node->type == TOKEN_FUNCTION_CALL) {
        fprintf(out, " <span class='node-val'>Target: ");
        fprintf_escaped(out, &pool->data[node->data.string_offset]);
        fprintf(out, "()</span>");
    } else if (node->type == TOKEN_ARG_LIST) {
        fprintf(out, " <span class='node-val'>Count: %d</span>", node->data.param.param_count);
    } 
    // --- ADDED: Descriptive text strings for your logical pipelines ---
    else if (node->type == TOKEN_AND) {
        fprintf(out, " <span class='node-val'>Operation: Short-Circuit Logical AND (&&)</span>");
    }
    else if (node->type == TOKEN_OR) {
        fprintf(out, " <span class='node-val'>Operation: Short-Circuit Logical OR (||)</span>");
    }
    else if (node->type == TOKEN_IF) {
        if (node->data.if_statement.false_block_idx != -1) {
            fprintf(out, " <span class='node-val'>Structure: If-Else Pipeline</span>");
        } else {
            fprintf(out, " <span class='node-val'>Structure: Simple If</span>");
        }
    }

    // Close the summary/content container
    if (has_children) {
        fprintf(out, "</summary>\n  <ul>\n");
    } else {
        fprintf(out, "</span>\n");
    }

    // Traversal Logic
    if (node->type == TOKEN_BLOCK) {
        int total = node->data.block.statement_count;
        for (int i = 0; i < total; i++) {
            int child_index = node->data.block.statement_indices[i];
            print_block_html(out, tree, &tree->nodes[child_index], pool, child_index);
        }
    } else if (node->type == TOKEN_PARAM) {
        int total = node->data.param.param_count;
        for (int i = 0; i < total; i++) {
            int child_index = node->data.param.param_indices[i]; 
            print_block_html(out, tree, &tree->nodes[child_index], pool, child_index);
        }
    } else if (node->type == TOKEN_ARG_LIST) {
        int total = node->data.param.param_count;
        for (int i = 0; i < total; i++) {
            int child_index = node->data.param.param_indices[i]; 
            print_block_html(out, tree, &tree->nodes[child_index], pool, child_index);
        }
    } 
    // --- ADDED: Specialized Traversal for the custom IfStatement Union format ---
    else if (node->type == TOKEN_IF) {
        // 1. Render the structural evaluation tree for the condition expressions
        if (node->data.if_statement.condition_idx != -1) {
            fprintf(out, "<li><span style='color: #eed49f; font-size: 0.9em; font-weight: bold;'>[Condition]</span>\n<ul>\n");
            print_block_html(out, tree, &tree->nodes[node->data.if_statement.condition_idx], pool, node->data.if_statement.condition_idx);
            fprintf(out, "</ul>\n</li>\n");
        }
        
        // 2. Render the body code execution block for the true path
        if (node->data.if_statement.true_block_idx != -1) {
            fprintf(out, "<li><span style='color: #a6e3a1; font-size: 0.9em; font-weight: bold;'>[Then Branch]</span>\n<ul>\n");
            print_block_html(out, tree, &tree->nodes[node->data.if_statement.true_block_idx], pool, node->data.if_statement.true_block_idx);
            fprintf(out, "</ul>\n</li>\n");
        }

        // 3. Render the fallback execution block or next cascading If node
        if (node->data.if_statement.false_block_idx != -1) {
            fprintf(out, "<li><span style='color: #f38ba8; font-size: 0.9em; font-weight: bold;'>[Else Branch]</span>\n<ul>\n");
            print_block_html(out, tree, &tree->nodes[node->data.if_statement.false_block_idx], pool, node->data.if_statement.false_block_idx);
            fprintf(out, "</ul>\n</li>\n");
        }
    } 
    else {
        // Fallback standard binary node evaluation (.left and .right)
        if (node->left != -1)  print_block_html(out, tree, &tree->nodes[node->left], pool, node->left);
        if (node->right != -1) print_block_html(out, tree, &tree->nodes[node->right], pool, node->right);
    }

    // Close open HTML groupings
    if (has_children) {
        fprintf(out, "  </ul>\n  </details>\n");
    }
    fprintf(out, "</li>\n");
}

void generate_ast_html(const char *filename, FileRegistry *registry, ASTTree *tree, StringPool *pool) {
    FILE *out = fopen(filename, "w");
    if (!out) {
        perror("Failed to open output HTML file");
        return;
    }

    // Write the HTML header and CSS styling (Included styles for the global container)
    fprintf(out, "<!DOCTYPE html>\n<html>\n<head>\n<style>\n");
    fprintf(out, ".node-TOKEN_FUNCTION_CALL { background: #89dceb; color: #11111b; }\n"); // Sky Blue Call Node
    fprintf(out, ".node-TOKEN_ARG_LIST { background: #94e2d5; color: #11111b; }\n");      // Pastel Teal Args Holder
    fprintf(out, "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #1e1e2e; color: #cdd6f4; padding: 20px; }\n");
    fprintf(out, "h1 { color: #f5c2e7; border-bottom: 2px solid #45475a; padding-bottom: 10px; }\n");
    fprintf(out, "h2 { color: #b4befe; margin-top: 20px; font-size: 1.2em; border-left: 3px solid #b4befe; padding-left: 8px; }\n");
    fprintf(out, "ul { list-style-type: none; padding-left: 24px; position: relative; }\n");
    fprintf(out, "ul::before { content: ''; position: absolute; left: 10px; top: 0; bottom: 0; width: 1px; background: #45475a; }\n");
    fprintf(out, "li { margin: 6px 0; position: relative; }\n");
    fprintf(out, "li::before { content: ''; position: absolute; left: -14px; top: 12px; width: 10px; height: 1px; background: #45475a; }\n");
    fprintf(out, "summary { cursor: pointer; padding: 4px 8px; background: #313244; border-radius: 4px; display: inline-block; transition: background 0.2s; }\n");
    fprintf(out, "summary:hover { background: #45475a; }\n");
    fprintf(out, ".leaf-content { padding: 4px 8px; background: #181825; border-radius: 4px; display: inline-block; }\n");
    fprintf(out, ".node-tag { font-family: monospace; font-weight: bold; font-size: 0.85em; padding: 2px 6px; border-radius: 3px; background: #89b4fa; color: #11111b; }\n");
    fprintf(out, ".node-val { font-family: monospace; color: #a6e3a1; margin-left: 8px; }\n");
    
    // Style accent for the scope badge (Now in use!)
    fprintf(out, ".node-scope { font-family: monospace; font-size: 0.8em; padding: 2px 5px; border-radius: 3px; background: #45475a; color: #a6adc8; margin-left: 6px; border: 1px solid #585b70; }\n");

    // Custom type badge accents
    fprintf(out, ".node-TOKEN_FUNCTION { background: #cba6f7; }\n");
    fprintf(out, ".node-TOKEN_BLOCK { background: #fab387; }\n");
    fprintf(out, ".node-TOKEN_GLOBAL_VAR { background: #f9e2af; }\n"); // Accent color for global variables
    
    // Color styling accent for the IF syntax node layout
    fprintf(out, ".node-TOKEN_IF { background: #f38ba8; color: #11111b; }\n"); 
    fprintf(out, ".node-TOKEN_AND { background: #f9e2af; color: #11111b; }\n"); // Pastel Yellow-Gold
    fprintf(out, ".node-TOKEN_OR { background: #fab387; color: #11111b; }\n");  // Soft Orange
    
    fprintf(out, "</style>\n</head>\n<body>\n");

    fprintf(out, "<h1>AST Visualizer</h1>\n");
    fprintf(out, "<p style='color: #a6adc8;'>File: %s</p>\n", registry->filename ? registry->filename : "Unknown");

    // ----------------------------------------------------
    // Section 1: Global Variables
    // ----------------------------------------------------
    fprintf(out, "<h2>Global Variables (%d)</h2>\n", registry->global_var_count);
    if (registry->global_var_count == 0) {
        fprintf(out, "<p style='font-style: italic; color: #585b70; padding-left: 10px;'>No global variables declared.</p>\n");
    } else {
        fprintf(out, "<div class='tree'>\n<ul>\n");
        for (int i = 0; i < registry->global_var_count; i++) {
            int node_index = registry->global_variables[i];
            print_block_html(out, tree, &tree->nodes[node_index], pool, node_index);
        }
        fprintf(out, "</ul>\n</div>\n");
    }

    // ----------------------------------------------------
    // Section 2: Global Functions
    // ----------------------------------------------------
    fprintf(out, "<h2>Functions (%d)</h2>\n", registry->function_count);
    if (registry->function_count == 0) {
        fprintf(out, "<p style='font-style: italic; color: #585b70; padding-left: 10px;'>No functions declared.</p>\n");
    } else {
        fprintf(out, "<div class='tree'>\n<ul>\n");
        for (int i = 0; i < registry->function_count; i++) {
            int node_index = registry->global_functions[i];
            
            if (node_index == registry->main_function_node_index) {
                fprintf(out, "<div style='border: 1px dashed #a6e3a1; padding: 4px; border-radius: 6px; margin-bottom: 8px;'>\n");
                fprintf(out, "<span style='font-size: 0.8em; color: #a6e3a1; font-weight: bold; margin-left: 10px;'>[Program Entry Point]</span>\n");
            }

            print_block_html(out, tree, &tree->nodes[node_index], pool, node_index);

            if (node_index == registry->main_function_node_index) {
                fprintf(out, "</div>\n");
            }
        }
        fprintf(out, "</ul>\n</div>\n");
    }

    fprintf(out, "</body>\n</html>\n");
    fclose(out);
    printf("AST visualizer file successfully generated: %s\n", filename);
}

int parse_factor(ASTTree *tree, TokenList *list, int *i, StringPool *pool);
int parse_term(ASTTree *tree, TokenList *list, int *i, StringPool *pool);
int parse_expression(ASTTree *tree, TokenList *list, int *i, StringPool *pool);
int parse_comparisons_ops(ASTTree *tree, TokenList *list, int *i, StringPool *pool);
int parse_logical_and(ASTTree *tree, TokenList *list, int *i, StringPool *pool);
int parse_body(ASTTree *tree, TokenList *list, StringPool *pool, int *i);

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
    tree->nodes[tree->count].scope_id = -1;
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
        case TOKEN_FUNCTION_CALL:
        case TOKEN_AND:
        case TOKEN_OR:
            tree->nodes[tree->count].data.string_offset = token.value_offset;
            break;

        case TOKEN_VAR_DECL:
        case TOKEN_UNKNOWN:
        case TOKEN_EOF:
        case TOKEN_BLOCK:
        case TOKEN_PARAM:
        case TOKEN_ARG_LIST:
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

int parse_argument_list(ASTTree *tree, TokenList *list, StringPool *pool, int *i) {
    Token arg_list_token = { .type = TOKEN_ARG_LIST, .value_offset = 0 };
    int arg_list_node = create_node(tree, arg_list_token, pool);

    int capacity = 8;
    int count = 0;
    int *arguments = malloc(capacity * sizeof(int));
    if (!arguments) { printf("Arguments failed to allocate. Out of memory\n"); exit(1); }
    while (*i < list->count && list->items[(*i)].type != TOKEN_RPAREN) {
        if (count >= capacity) {
            capacity *= 2;
            
            int *temp = realloc(arguments, capacity * sizeof(int));
            if (!temp) { printf("Arguments failed to reallocate. Out of memory\n"); exit(1); }
            arguments = temp;
        }
        
        int expr_node_idx =  parse_expression(tree, list, i, pool);
        if (expr_node_idx == -1) {
            if (count > 0) {
                printf("Expected expression in argument list at index %d line %d column %d\n",
                    (*i),
                    list->items[*i].line,
                    list->items[*i].column);
                exit(1);
            }
            break;
        }
        
        arguments[count++] = expr_node_idx;
        
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
        arguments = realloc(arguments, count *sizeof(int));
    } else {
        free(arguments);
        arguments = NULL;
    }

    tree->nodes[arg_list_node].data.param.param_indices = arguments;
    tree->nodes[arg_list_node].data.param.param_count = count;
    
    return arg_list_node;
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

        case 'c':
            safe_set_right(tree, current_node, parse_comparisons_ops(tree, list, i, pool));
            break;

        case 'a':
            safe_set_right(tree, current_node, parse_logical_and(tree, list, i, pool));
            break;
        
        default:
            printf("Unknown mode");
            exit(1);
            break;
    }

    return current_node;
}

int parse_factor(ASTTree *tree, TokenList *list, int *i, StringPool *pool) {
    Token current_token = list->items[(*i)];

    if (current_token.type == TOKEN_INT_LIT || current_token.type == TOKEN_CHAR_STRING) {
        int current_node = create_node(tree, list->items[*i], pool);
        (*i)++;

        return current_node;
    }
    
    if (current_token.type == TOKEN_IDENTIFIER) {
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

            return call_node;
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

int parse_comparisons_ops(ASTTree *tree, TokenList *list, int *i, StringPool *pool) {
    int current_node = parse_expression(tree, list, i, pool);

    if (list->items[(*i)].type == TOKEN_COMPARE_EQ || list->items[(*i)].type == TOKEN_NOT_EQ ||
        list->items[(*i)].type == TOKEN_GREATER_THAN || list->items[(*i)].type == TOKEN_LESS_THAN_OR_EQ ||
        list->items[(*i)].type == TOKEN_LESS_THAN || list->items[(*i)].type == TOKEN_GREATER_THAN_OR_EQ) {
        current_node = merge_node(tree, list, i, current_node, 'e', pool);
    }

    return current_node;
}

int parse_logical_and(ASTTree *tree, TokenList *list, int *i, StringPool *pool) {
    int current_node = parse_comparisons_ops(tree, list, i, pool);

    while (list->items[(*i)].type == TOKEN_AND) {
        current_node = merge_node(tree, list, i, current_node, 'c', pool);
    }
    
    return current_node;
}

int parse_logical_or(ASTTree *tree, TokenList *list, int *i, StringPool *pool) {
    int current_node = parse_logical_and(tree, list, i, pool);

    while (list->items[(*i)].type == TOKEN_OR) {
        current_node = merge_node(tree, list, i, current_node, 'a', pool);
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
            if (peek(list, *i, 1) == TOKEN_ASSIGN) {
                Token ident_token = list->items[(*i)];
                (*i)++;

                current_node = create_node(tree, list->items[(*i)], pool);
                (*i)++;

                safe_set_left(tree, current_node, create_node(tree, ident_token, pool));
                safe_set_right(tree, current_node, parse_expression(tree, list, i, pool));

                check_semicolon(list->items, list->items[(*i)].column, list->items[(*i)].line, i);
            } else if (peek(list, *i, 1) == TOKEN_LPAREN) {
                current_node = parse_factor(tree, list, i, pool);
                
                check_semicolon(list->items, list->items[(*i)].column, list->items[(*i)].line, i);
            } else {
                printf("Error: Bare identifier statement not allowed at line %d column %d\n",
                    list->items[(*i)].line,
                    list->items[(*i)].column);
                exit(1);
            }

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

            int comparisons_index = parse_logical_or(tree, list, i, pool);

            if (list->items[(*i)].type != TOKEN_RPAREN) {
                printf("Missing parentheses at index %d line %d column %d",
                    (*i),
                    list->items[(*i)].line,
                    list->items[(*i)].column);
                exit(1);
            }
            (*i)++;

            tree->nodes[current_node].data.if_statement.condition_idx = comparisons_index;

            if (list->items[(*i)].type != TOKEN_LBRACES) {
                printf("Missing braces at index %d line %d column %d",
                    (*i),
                    list->items[(*i)].line,
                    list->items[(*i)].column);
                exit(1);
            }
            (*i)++;
            
            int body_block_index = parse_body(tree, list, pool, i);
            tree->nodes[current_node].data.if_statement.true_block_idx = body_block_index;
            
            if (list->items[(*i)].type != TOKEN_RBRACES) {
                printf("Missing braces at index %d line %d column %d",
                    (*i),
                    list->items[(*i)].line,
                    list->items[(*i)].column);
                exit(1);
            }
            (*i)++;

            if (list->items[(*i)].type == TOKEN_ELSE) {
                (*i)++;

                int false_block_idx;
                if (list->items[(*i)].type == TOKEN_IF) {
                    false_block_idx = parse_block(list, pool, i, tree); 
                } else {
                    if (list->items[(*i)].type != TOKEN_LBRACES) {
                        printf("Missing braces at index %d line %d column %d",
                            (*i),
                            list->items[(*i)].line,
                            list->items[(*i)].column);
                        exit(1);
                    }
                    (*i)++;

                    false_block_idx = parse_body(tree, list, pool, i);
                    
                    if (list->items[(*i)].type != TOKEN_RBRACES) {
                        printf("Missing braces at index %d line %d column %d",
                            (*i),
                            list->items[(*i)].line,
                            list->items[(*i)].column);
                        exit(1);
                    }
                    (*i)++;
                }
                
                tree->nodes[current_node].data.if_statement.false_block_idx = false_block_idx;
                break;
            } else {
                tree->nodes[current_node].data.if_statement.false_block_idx = -1;
                break;
            }
            
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
        statements = realloc(statements, count * sizeof(int));
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

int parse_program(FileRegistry *registry, ASTTree *tree, TokenList *list, StringPool *pool, int *i) {
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

void ensure_registry_capacity(FileRegistry *registry) {
    if (registry->function_count >= registry->function_capacity) {
        registry->function_capacity *= 2;

        int *temp = realloc(registry->global_functions, registry->function_capacity * sizeof(int));
        if (!temp) {
            printf("func registry failed to reallocate. Out of memory");
            exit(1);
        }
        registry->global_functions = temp;
    }
    
    if (registry->global_var_count >= registry->global_var_capacity) {
        registry->global_var_capacity *= 2;

        int *temp = realloc(registry->global_variables, registry->global_var_capacity * sizeof(int));
        if (!temp) {
            printf("global var registry failed to reallocate. Out of memory");
            exit(1);
        }
        registry->global_variables = temp;
    }
}

void parsing(TokenList *list, FileRegistry *registry, StringPool *pool, ASTTree *tree) {
    if (list == NULL || list->items == NULL) {
        printf("Error");
        exit(1);
    }

    int i = 0;
    while (i < list->count && list->items[i].type != TOKEN_EOF) {
        ensure_registry_capacity(registry);
        if (peek(list, i, 2) == TOKEN_ASSIGN || peek(list, i, 2) == TOKEN_SEMICOLON) {
            registry->global_variables[registry->global_var_count++] = parse_block(list, pool, &i, tree);
        } else if (peek(list, i, 2) == TOKEN_LPAREN) {
            registry->global_functions[registry->function_count++] = parse_program(registry, tree, list, pool, &i);
        }
    }
}
