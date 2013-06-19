#ifndef IR_H
#define IR_H

#define IR_ARG_CALL  0x1
#define IR_ARG_PARAM 0x2
#define IR_ARG_WORD  0x3
#define IR_ARG_DATA  0x4

#include <stdlib.h>

struct ir_func;

struct ir_type {
   struct ir_func* add;
   struct ir_func* sub;
   struct ir_func* mul;
   struct ir_func* div;
};

struct ir_arg {
   struct ir_type* type;
   int arg_type;
   union {
      int word;
      struct ir_param* param;
      struct {
         struct ir_func* func;
         struct ir_arg* arg;
      } call;
      struct {
         void* ptr;
         size_t size;
      } data;
   };
   struct ir_arg* next;
};

struct ir_param {
   char* name;
   int index;
   struct ir_param* next;
};

struct ir_func {
   char* name;
   struct ir_param* param;
   struct ir_arg* value;
   struct ir_func* next;
};


#endif
