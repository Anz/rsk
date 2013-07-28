#ifndef X86_H
#define X86_H

#include "map.h"
#include "ir.h"
#include "elf.h"
#include "buffer.h"
#include <string.h>

struct nr {
   symbol_t* symbols;
   struct buffer text;
   struct buffer data;
};

struct nr x86_compile(struct map funcs);

#endif

