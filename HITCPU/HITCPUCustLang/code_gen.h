#ifndef CODE_GEN_HEADER_H
#define CODE_GEN_HEADER_H

#include "struct.h"

void hitcpu_emit_assembly(const char *output_filename, InstructionStream *stream, StringPool *pool, RegStatus *reg_status);

#endif