#ifndef IR_GEN_HEADER_H
#define IR_GEN_HEADER_H

#include "struct.h"

void generate_ir(FileRegistry *registry, ASTTree *tree, StringPool *pool, InstructionStream *stream);
void dump_stream_to_html(const char *filename, InstructionStream *stream, SymbolLists *sym_lists, StringPool *string_pool);

#endif