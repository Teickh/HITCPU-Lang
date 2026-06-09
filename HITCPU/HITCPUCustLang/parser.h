#ifndef PARSER_HEADER_H
#define PARSER_HEADER_H

#include <stdio.h>

#include "struct.h"

void parsing(TokenList *list, FileRegistry *registry, StringPool *pool, ASTTree *tree);
void print_ast(FileRegistry *registry, ASTTree *tree, StringPool *pool, int level);

#endif