#ifndef CODE_GEN_HEADER_H
#define CODE_GEN_HEADER_H

#include <stdio.h>

#include "struct.h"

void generate_code(FileRegistry *registry, ASTTree *tree, StringPool *pool, InstructionStream *stream);
void dump_stream_to_html(const char *filename, InstructionStream *stream, SymbolLists *sym_lists, StringPool *string_pool);

#endif