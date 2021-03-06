#ifndef ELF_H
#define ELF_H

#include "buffer.h"
#include <stdio.h>

typedef struct symbol {
   char* name;
   int text;
   size_t text_size;
   struct symbol* next;
} symbol_t;

void elf_write(FILE* fd, symbol_t symbols, struct buffer* text, struct buffer* data);

#endif
