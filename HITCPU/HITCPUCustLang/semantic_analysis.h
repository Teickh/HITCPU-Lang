#ifndef SEMANTIC_ANALYSIS_HEAHER_H
#define SEMANTIC_ANALYSIS_HEAHER_H

#include "struct.h"

void analyse(FileRegistry *registry, ASTTree *tree, ScopeStack *current_scope);

#endif