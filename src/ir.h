#ifndef IR_H
#define IR_H

#define IR_ARG_CALL  0x1
#define IR_ARG_PARAM 0x2
#define IR_ARG_DATA  0x3

#define IR_ERR_BIN_OP_NE 0x1 // binary operands are not of the same type
#define IR_ERR_NR_ARGS   0x2 // number of arguments wrong (too few/much) 

#include "list.h"
#include "map.h"
#include <stdlib.h>

struct ir_func;
struct ir_param;

struct ir_type {
   char* name;
   struct map ops;
};

struct ir_param {
   char* name;
   int index;
   int lineno;
};

struct ir_arg {
   int arg_type;
   union {
      struct {
         union {
            struct ir_param* param;
            struct ir_func* func;
         };
         struct list args;
      } call;
      struct {
         union {
            int word;
            void* ptr;
         };
         size_t size;
         struct ir_type* type;
      } data;
   };
   
   int lineno;
};

struct ir_case {
   struct ir_arg* cond;
   struct ir_arg* func;
   int lineno;
};

struct ir_func {
   char* name;
   struct map params;
   struct list cases;
   struct ir_type* type;
   int lineno;
   int ref;
};

struct ir_error {
   int code;
   struct ir_func* func;
   struct ir_arg* arg;
   int lineno;
};

// construction
struct ir_func* ir_func_init(char* name, int lineno);
void ir_func_clear(struct ir_func* f);
struct ir_case* ir_func_case(struct ir_arg* cond, struct ir_arg* func, int lineno);
struct ir_param* ir_func_param(struct ir_func* func, char* name, int lineno);
struct ir_arg* ir_arg_op(struct ir_func* func, struct ir_arg* a, struct ir_arg* b, int lineno);
struct ir_arg* ir_arg_word(int word, struct ir_type* type, int lineno);
struct ir_arg* ir_arg_data(char* ptr, size_t size, struct ir_type* type, int lineno);
struct ir_arg* ir_arg_call(struct map* funcs, struct ir_func* func, char* name, struct list* args, int lineno);

// copy
struct ir_func* ir_func_cpy(struct ir_func* f);
void ir_params_cpy(struct ir_func* f, struct map params);
struct ir_arg* ir_arg_cpy(struct ir_func* f, struct ir_arg* a);
struct list ir_cases_cpy(struct ir_func* f, struct list cases);

// error function
void ir_print_err(struct ir_error err);

#endif
