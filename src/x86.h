#ifndef X86_H
#define X86_H

#include "ir.h"
#include "elf.h"
#include "buffer.h"
#include <string.h>

struct nr {
   symbol_t* symbols;
   char* text;
   size_t text_size;
   struct buffer data;
};

struct nr x86_compile(struct ir_func* funcs);

#endif

