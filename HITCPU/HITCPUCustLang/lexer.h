#ifndef LEXER_HEADER_H
#define LEXER_HEADER_H

#include <stdio.h>

#include "struct.h"

// Only the public API is exposed here
void lexing(FILE *program, TokenList *list, StringPool *pool, int *total_lines);
void print_tokens(TokenList *list, StringPool *pool);

#endif