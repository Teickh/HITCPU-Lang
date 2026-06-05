#ifndef PARSER_HEADER_H
#define PARSER_HEADER_H

#include <stdio.h>

#include "struct.h"

void parsing(TokenList *list, ProgramRegistry *program, StringPool *pool, ASTTree *tree, int *program_root);
void print_ast(ProgramRegistry *registry, ASTTree *tree, StringPool *pool, int level);

#endif