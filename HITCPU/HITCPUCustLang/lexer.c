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

void print_tokens(TokenList *list, StringPool *pool) {
    printf("\n--- Captured Tokens ---\n");
    printf("%-10s | %-15s | %-10s | %-5s | %-5s\n", "Index", "Token Type", "Value", "Line", "Column");
    printf("-----------------------------------------------------\n");

    for (int i = 0; i < list->count; i++) {
        Token t = list->items[i];
        const char *token_next = &pool->data[t.value_offset];
        printf("%-10d | %-15s | %-10s | %-5d | %-5d\n", i, token_type_to_string(t.type), token_next, t.line, t.column);
    }
}

void add_token(TokenList *list, StringPool *pool, TokenType type, const char *value, int line, int column) {
    int offset = find_string_in_pool(pool, value);
    
    if (list->count >= list->capacity) {
        list->capacity = (list->capacity == 0) ? 8 : list->capacity * 2;

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
        while (pool->size + 1 >= pool->capacity) {
            pool->capacity = (pool->capacity == 0) ? 1024 : pool->capacity * 2;

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
            if (c == '\n') (*total_lines)++;
            continue;
        }
        
        if (isalpha(c)) {
            char buffer[1024];
            int j = 0;

            buffer[j++] = (char) c;
            while (isalnum(c = fgetc(program))) if (j < 1023) buffer[j++] = (char) c;
            ungetc(c, program);
            
            buffer[j] = '\0';

            add_token(list, pool, check_keyword(buffer), buffer, *total_lines, column);
        } else if (isdigit(c)) {
            char buffer[1024];
            int j = 0;
            
            buffer[j++] = (char) c;
            while (isdigit(c = fgetc(program))) if (j < 1023) buffer[j++] = (char) c;
            ungetc(c, program);

            buffer[j] = '\0';

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
                case '/': add_token(list, pool, TOKEN_DIVIDE, "/", *total_lines, column); break;
                case '(': add_token(list, pool, TOKEN_LPAREN, "(", *total_lines, column); break;
                case ')': add_token(list, pool, TOKEN_RPAREN, ")", *total_lines, column); break;
                case '>': add_token(list, pool, TOKEN_GREATER_THAN, ">", *total_lines, column); break;
                case '<': add_token(list, pool, TOKEN_LESS_THAN, "<", *total_lines, column); break;
                case '{': add_token(list, pool, TOKEN_LBRACES, "{", *total_lines, column); break;
                case '}': add_token(list, pool, TOKEN_RBRACES, "}", *total_lines, column); break;
                case '[': add_token(list, pool, TOKEN_LBRACKET, "[", *total_lines, column); break;
                case ']': add_token(list, pool, TOKEN_RBRACKET, "]", *total_lines, column); break;
                case ',': add_token(list, pool, TOKEN_COMMA, ",", *total_lines, column); break;
                case ';': 
                    add_token(list, pool, TOKEN_SEMICOLON, ";", *total_lines, column);
                    column = -1;
                    break;
                case '"': {
                    char buffer[1024];
                    int j = 0;
                    int next_char;

                    while ((next_char = fgetc(program)) != '"' && next_char != EOF) {
                        if (j < 1023) {
                            buffer[j++] = (char)next_char;
                        }
                    }

                    if (next_char == EOF) {
                        printf("Missing double quotes at line %d\n", *total_lines + 1);
                        exit(1);
                    }
                    
                    buffer[j] = '\0';

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
