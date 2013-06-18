#ifndef X86_H
#define X86_H

#include "ir.h"
#include "elf.h"
#include <string.h>

struct nr {
   symbol_t* symbols;
   char* text;
};

struct nr x86_compile(struct func* funcs);

#endif

