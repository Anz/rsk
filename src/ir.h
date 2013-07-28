#ifndef IR_H
#define IR_H

#define IR_ARG_CALL  0x1
#define IR_ARG_PARAM 0x2
#define IR_ARG_WORD  0x3
#define IR_ARG_DATA  0x4

#define IR_RES_UNK   0x0
#define IR_RES_DYN   0x1
#define IR_RES_STA   0x2

#include "list.h"
#include "map.h"
#include <stdlib.h>

struct ir_func;

struct ir_error {
   int lineno;
   char* msg;
};

struct ir_type {
   struct ir_func* add;
   struct ir_func* sub;
   struct ir_func* mul;
   struct ir_func* div;
};

struct ir_var_type {
   int type;
   union {
      struct ir_type* sta;
      int dyn;
   };
};

struct ir_arg {
   int arg_type;
   union {
      int word;
      int param;
      struct {
         struct ir_func* func;
         struct list args;
      } call;
      struct {
         void* ptr;
         size_t size;
      } data;
   };
   
   struct ir_var_type type;
};

struct ir_func {
   char* name;
   struct map params;
   struct ir_arg* value;
};


#endif
