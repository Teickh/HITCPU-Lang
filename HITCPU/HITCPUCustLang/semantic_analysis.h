#ifndef SEMANTIC_ANALYSIS_HEAHER_H
#define SEMANTIC_ANALYSIS_HEAHER_H

#include "struct.h"

void analyse(FileRegistry *registry, ASTTree *tree, ScopeStack *scope_stack, SymbolLists *symbol_lists);

#endif