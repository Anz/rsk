#ifndef ASM_H
#define ASM_H

#include "buffer.h"

void asm_init();
void asm_free();
void asm_x86(struct buffer* buf, char* str);

#endif
