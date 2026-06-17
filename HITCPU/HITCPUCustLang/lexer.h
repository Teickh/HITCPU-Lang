#ifndef LEXER_HEADER_H
#define LEXER_HEADER_H


#include "struct.h"

void lexing(FILE *program, TokenList *list, StringPool *pool, int *total_lines);
void print_tokens_to_html(TokenList *list, StringPool *pool, const char *filename);
void print_string_pool_to_html(StringPool *pool, const char *filename);

#endif