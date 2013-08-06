#include "asm.h"
#include "map.h"
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>

#define ASM_OP_REG_REG  0x1
#define ASM_OP_REG_MEM  0x2
#define ASM_OP_MEM_REG  0x3
#define ASM_OP_VAL_REG  0x4

const int MAX_GROUPS = 10;

struct asm_reg {
   int num;
};

struct asm_op {
   int code;
};

regex_t pattern;

void asm_init() {
   int errcode = regcomp(&pattern, "([A-Za-z]+) +(%[A-Za-z]+|\\(%[A-Za-z]+\\)|\\$[0-9x]+) *(, *(%[A-Za-z]+|\\(%[A-Za-z]+\\)|\\$[0-9x]+) *)?;", REG_EXTENDED);
   
   if (errcode) {
      printf("asm: regex compile issue\n");
   }
}

void asm_free() {
   regfree(&pattern);
}

static struct asm_reg* asm_reg(int num) {
   struct asm_reg* reg = malloc(sizeof(*reg));
   memset(reg, 0, sizeof(*reg));
   reg->num = num;
   return reg;
}

static struct asm_op* asm_op(int code) {
   struct asm_op* op = malloc(sizeof(*op));
   memset(op, 0, sizeof(*op));
   op->code = code;
   return op;
}

static void asm_x86_enc(struct buffer* buf, struct asm_op* op, struct asm_reg* r1, struct asm_reg* r2) {
      char ins = ((char)op->code) << 2;
      char s = 0x01;
      char d = ~0x02;
      
      buffer_writeb(buf, (ins & d) |  s);
      
      char mod_reg_rm = 0xC0 | (r1->num << 3) | (r2->num);
      buffer_writeb(buf, mod_reg_rm);
}

void asm_x86(struct buffer* buf, char* str) {
   struct map reg;
   map_init(&reg);
   map_set(&reg, "eax", 3, asm_reg(0x00));
   map_set(&reg, "ecx", 3, asm_reg(0x01));
   map_set(&reg, "edx", 3, asm_reg(0x02));
   map_set(&reg, "ebx", 3, asm_reg(0x03));
   map_set(&reg, "esp", 3, asm_reg(0x04));
   map_set(&reg, "ebp", 3, asm_reg(0x05));
   map_set(&reg, "esi", 3, asm_reg(0x06));
   map_set(&reg, "edi", 3, asm_reg(0x07));
   
   struct map ops;
   map_init(&ops);
   map_set(&ops, "add", 3, asm_op(0x0));
   

   regmatch_t groups[MAX_GROUPS];
   
   if (regexec(&pattern, str, MAX_GROUPS, groups, 0) == 0) {
      int mode;
      struct asm_op* op = map_get(&ops, str+groups[1].rm_so, groups[1].rm_eo-groups[1].rm_so);
      struct asm_reg* r1 = NULL;
      struct asm_reg* r2 = map_get(&reg, str+groups[4].rm_so+1, groups[4].rm_eo-groups[4].rm_so-1);
      int val;
      
      switch (str[groups[2].rm_so]) {
         case '%':
            printf("arg1 = register\n");
            r1 = map_get(&reg, str+groups[2].rm_so+1, groups[2].rm_eo-groups[2].rm_so-1);
            break;
         case '(': 
            printf("arg1 = address (not implemented yet)\n"); 
            break;
         case '$':
            mode = ASM_OP_VAL_REG;
            //val = atoi();
            printf("arg1 = value\n"); 
            break;
      }
      
      printf("found: %.*s\n", groups[2].rm_eo-groups[2].rm_so, str+groups[2].rm_so);
      printf("ins: %02i\n", op->code);
      printf("regs: %p %p\n", r1, r2);
      
      asm_x86_enc(buf, op, r1, r2);
 
    // Go over all matches. A match with rm_so = -1 signals the end
    for (size_t i = 0; i < MAX_GROUPS; ++i) {
      if (groups[i].rm_so == -1)
        break;
      printf("Match group %zu: ", i);
      for (regoff_t p = groups[i].rm_so;
           p < groups[i].rm_eo; ++p) {
        putchar(str[p]);
      }
      putchar('\n');
    }
  } else {
     printf("not found pattern\n");
  }
  
  
  // clean ops
  for (map_it* it = map_iterator(&ops); it != NULL; it = map_next(it)) {
      free(it->data);
  }
  map_clear(&ops);
  
  // clean regs
  for (map_it* it = map_iterator(&reg); it != NULL; it = map_next(it)) {
      free(it->data);
  }
  map_clear(&reg);
}
