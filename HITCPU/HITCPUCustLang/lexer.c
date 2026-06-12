#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"
#include "struct.h"

int find_string_in_pool(StringPool *pool, const char *value) {
    int offset = 0;
    while (offset < pool->size) {
        if (strcmp(&pool->data[offset], value) == 0) {
            return offset;
        }
        offset += strlen(&pool->data[offset]) + 1;
    }
    return -1;
}

void print_tokens_to_html(TokenList *list, StringPool *pool, const char *filename) {
    FILE *html = fopen(filename, "w");
    if (!html) {
        fprintf(stderr, "Error: Could not create HTML file %s\n", filename);
        return;
    }

    // Write HTML Boilerplate and CSS styling
    fprintf(html, "<!DOCTYPE html>\n<html>\n<head>\n<title>Lexer Output</title>\n");
    fprintf(html, "<style>\n");
    fprintf(html, "  body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 30px; background-color: #f7f9fa; color: #333; }\n");
    fprintf(html, "  h2 { color: #2c3e50; border-bottom: 2px solid #34495e; padding-bottom: 8px; margin-top: 40px; }\n");
    fprintf(html, "  table { width: 100%%; border-collapse: collapse; margin-top: 15px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); background-color: #fff; }\n");
    fprintf(html, "  th, td { padding: 12px 15px; text-align: left; border: 1px solid #e0e0e0; }\n");
    fprintf(html, "  th { background-color: #34495e; color: #ffffff; font-weight: 600; }\n");
    fprintf(html, "  tr:nth-child(even) { background-color: #f2f4f4; }\n");
    fprintf(html, "  tr:hover { background-color: #eaf2f8; }\n");
    fprintf(html, "  .mono { font-family: 'Courier New', Courier, monospace; font-weight: bold; }\n");
    fprintf(html, "</style>\n</head>\n<body>\n");

    // 1. Captured Tokens Table
    fprintf(html, "<h2>Captured Tokens</h2>\n");
    fprintf(html, "<table>\n  <tr>\n    <th>Index</th>\n    <th>Token Type</th>\n    <th>Value</th>\n    <th>Line</th>\n    <th>Column</th>\n  </tr>\n");

    for (int i = 0; i < list->count; i++) {
        Token t = list->items[i];
        const char *token_val = &pool->data[t.value_offset];
        
        fprintf(html, "  <tr>\n");
        fprintf(html, "    <td>%d</td>\n", i);
        fprintf(html, "    <td>%s</td>\n", token_type_to_string(t.type));
        // Using class "mono" to make raw token values stand out clearly
        fprintf(html, "    <td class=\"mono\">%s</td>\n", token_val); 
        fprintf(html, "    <td>%d</td>\n", t.line);
        fprintf(html, "    <td>%d</td>\n", t.column);
        fprintf(html, "  </tr>\n");
    }
    fprintf(html, "</table>\n");

    // 2. String Pool Table
    fprintf(html, "<h2>String Pool Status</h2>\n");
    fprintf(html, "<p><strong>Pool Size:</strong> %d / <strong>Pool Capacity:</strong> %d bytes</p>\n", pool->size, pool->capacity);
    fprintf(html, "<table>\n  <tr>\n    <th style=\"width: 15%%;\">Offset</th>\n    <th>String Value</th>\n  </tr>\n");

    int offset = 0;
    while (offset < pool->size) {
        const char *pool_str = &pool->data[offset];
        fprintf(html, "  <tr>\n");
        fprintf(html, "    <td>%d</td>\n", offset);
        fprintf(html, "    <td class=\"mono\">\"%s\"</td>\n", pool_str);
        fprintf(html, "  </tr>\n");
        
        offset += strlen(pool_str) + 1;
    }
    fprintf(html, "</table>\n");

    fprintf(html, "\n</body>\n</html>\n");
    fclose(html);
    printf("HTML visualizer file successfully generated: %s\n", filename);
}

void add_token(TokenList *list, StringPool *pool, TokenType type, const char *value, int line, int column) {
    int offset = find_string_in_pool(pool, value);
    
    if (list->count >= list->capacity) {
        list->capacity *= 2;

        Token *temp = realloc(list->items, list->capacity * sizeof(Token));
        if (!temp) {
            printf("Token realloc failed. Out of memory");
            exit(1);
        }
        
        list->items = temp;
    }

    if (offset == -1) {
        offset = pool->size;

        int len = strlen(value) + 1;
        if (pool->size + len >= pool->capacity) {
            pool->capacity *= 2;

            char *temp = realloc(pool->data, pool->capacity);
            if (!temp) {
                printf("Out of memory");
                exit(1);
            }
            pool->data = temp;
        }

        strcpy(&pool->data[pool->size], value);
        pool->size += len;
    }
    
    list->items[list->count].type = type;
    list->items[list->count].value_offset = offset;
    list->items[list->count].line = line + 1;
    list->items[list->count].column = column;
    list->count++;
}

TokenType check_keyword(const char *word) {
    if (strcmp(word, "int") == 0)
        return TOKEN_INT;
    
    else if (strcmp(word, "char") == 0)
        return TOKEN_CHAR;
    
    else if (strcmp(word, "if") == 0)
        return TOKEN_IF;
    
    else if (strcmp(word, "print") == 0)
        return TOKEN_PRINT;

    else if (strcmp(word, "return") == 0)
        return TOKEN_RETURN;
    
    return TOKEN_IDENTIFIER;
}

void lexing(FILE *program, TokenList *list, StringPool *pool, int *total_lines) {
    int column = 0;
    int c;
    while ((c = fgetc(program)) != EOF) {
        if (isspace(c)) {
            if (c == '\n') {
                (*total_lines)++; 
                column = 0;
            }
            continue;
        }
        
        if (isalpha(c)) {
            char buffer[1024];
            int j = 0;

            buffer[j++] = (char) c;
            while (isalnum(c = fgetc(program))) if (j < 1023) buffer[j++] = (char) c;
            ungetc(c, program);
            
            buffer[j] = '\0';

            if (j >= 1023) {
                fprintf(stderr, "Lexer error: Identifier too long at line %d\n", *total_lines);
                exit(1);
            }

            add_token(list, pool, check_keyword(buffer), buffer, *total_lines, column);
        } else if (isdigit(c)) {
            char buffer[1024];
            int j = 0;
            
            buffer[j++] = (char) c;
            while (isdigit(c = fgetc(program))) if (j < 1023) buffer[j++] = (char) c;
            ungetc(c, program);

            buffer[j] = '\0';

            if (j >= 1023) {
                fprintf(stderr, "Lexer error: Identifier too long at line %d\n", *total_lines);
                exit(1);
            }

            add_token(list, pool, TOKEN_INT_LIT, buffer, *total_lines, column);
        } else {
            int next = 0;
            switch (c) {
                case '=':
                    next = fgetc(program);
                    if (next == '=') {
                        add_token(list, pool, TOKEN_COMPARE_EQ, "==", *total_lines, column);
                    } else {
                        ungetc(next, program);
                        add_token(list, pool, TOKEN_ASSIGN, "=", *total_lines, column);
                    }
                    break;
                case '!':
                    next = fgetc(program);
                    if (next == '=') {
                        add_token(list, pool, TOKEN_NOT_EQ, "!=", *total_lines, column);
                    } else {
                        ungetc(next, program);
                        add_token(list, pool, TOKEN_UNKNOWN, "!", *total_lines, column);
                    }
                    break;
                case '+': add_token(list, pool, TOKEN_PLUS, "+", *total_lines, column); break;
                case '-': add_token(list, pool, TOKEN_MINUS, "-", *total_lines, column); break;
                case '*': add_token(list, pool, TOKEN_MULTIPLY, "*", *total_lines, column); break;
                case '(': add_token(list, pool, TOKEN_LPAREN, "(", *total_lines, column); break;
                case ')': add_token(list, pool, TOKEN_RPAREN, ")", *total_lines, column); break;
                case '>': add_token(list, pool, TOKEN_GREATER_THAN, ">", *total_lines, column); break;
                case '<': add_token(list, pool, TOKEN_LESS_THAN, "<", *total_lines, column); break;
                case '{': add_token(list, pool, TOKEN_LBRACES, "{", *total_lines, column); break;
                case '}': add_token(list, pool, TOKEN_RBRACES, "}", *total_lines, column); break;
                case '[': add_token(list, pool, TOKEN_LBRACKET, "[", *total_lines, column); break;
                case ']': add_token(list, pool, TOKEN_RBRACKET, "]", *total_lines, column); break;
                case ',': add_token(list, pool, TOKEN_COMMA, ",", *total_lines, column); break;
                case '/': {
                    next = fgetc(program);
                    if (next != '/') {
                        ungetc(next, program);
                        add_token(list, pool, TOKEN_DIVIDE, "/", *total_lines, column); break;
                    }

                    while ((next = fgetc(program)) != '\n' && next != EOF) {
                        column++;
                    }

                    break;
                }
                case ';': 
                    add_token(list, pool, TOKEN_SEMICOLON, ";", *total_lines, column);
                    column = -1;
                    break;
                case '"': {
                    char buffer[1024];
                    int j = 0;

                    while ((next = fgetc(program)) != '"' && next != EOF) {
                        if (j < 1023) {
                            buffer[j++] = (char)next;
                        }
                    }

                    if (next == EOF) {
                        printf("Missing double quotes at line %d\n", *total_lines + 1);
                        exit(1);
                    }
                    
                    buffer[j] = '\0';

                    if (j >= 1023) {
                        fprintf(stderr, "Lexer error: Identifier too long at line %d\n", *total_lines);
                        exit(1);
                    }

                    add_token(list, pool, TOKEN_CHAR_STRING, buffer, *total_lines, column);
                    break;
                }
                case '\'': {
                    char buffer[4];
                    int j = 0;
                    int next_char;

                    next_char = fgetc(program);
                    if (next_char != '\'' && next_char != EOF) {
                        buffer[j++] = (char)next_char;
                        
                        int close_quote = fgetc(program);
                        if (close_quote != '\'') {
                            ungetc(close_quote, program); 
                        }
                    }

                    if (j >= 1023) {
                        fprintf(stderr, "Lexer error: Identifier too long at line %d\n", *total_lines);
                        exit(1);
                    }

                    buffer[j] = '\0';
                    add_token(list, pool, TOKEN_CHAR_LIT, buffer, *total_lines, column);
                    break;
                }

                default: {
                    char unknwon_str[2] = { (char) c, '\0' };
                    add_token(list, pool, TOKEN_UNKNOWN, unknwon_str, *total_lines, column);
                    break;
                }
            }
        }

        column++;
    }

    add_token(list, pool, TOKEN_EOF, "EOF", *total_lines, column);
}
