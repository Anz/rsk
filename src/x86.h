#ifndef X86_H
#define X86_H

#include "ir.h"
#include "elf.h"
#include <string.h>

struct nr {
   symbol_t* symbols;
   char* text;
   size_t text_size;
   char* data;
   size_t data_size;
};

struct nr x86_compile(struct ir_func* funcs);

#endif

