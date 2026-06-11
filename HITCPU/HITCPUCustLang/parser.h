#ifndef PARSER_HEADER_H
#define PARSER_HEADER_H

#include <stdio.h>

#include "struct.h"

void parsing(TokenList *list, FileRegistry *registry, StringPool *pool, ASTTree *tree);
void generate_ast_html(const char *filename, FileRegistry *registry, ASTTree *tree, StringPool *pool);

#endif