#ifndef IR_H
#define IR_H

#define IR_ARG_CALL  0x1
#define IR_ARG_PARAM 0x2
#define IR_ARG_DATA  0x3

// error definitions
#define IR_ERR_BIN_OP_NE   0x1 // binary operands are not of the same type
#define IR_ERR_NR_ARGS     0x2 // number of arguments wrong (too few/much) 
#define IR_ERR_RET_TYPE_UN 0x3 // unknown return type
#define IR_ERR_RET_TYPE_NE 0x4 // return type not equal in all cases
#define IR_ERR_COND_TYPE   0x5 // return type of condition must be boolean

// error levels
#define IR_LVL_SOURCE   0x01
#define IR_LVL_SUBSEQ   0x02
#define IR_LVL_WARN     0x03

#include "list.h"
#include "map.h"
#include <stdlib.h>
#include <stdbool.h>

struct ir_func;
struct ir_param;

typedef list_t* ir_type_t;

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
            int param;
            char* func_name;
         };
         struct list args;
      } call;
      struct {
         union {
            int word;
            void* ptr;
         };
         size_t size;
         ir_type_t type;
      } data;
   };
   
   int lineno;
};

struct ir_case {
   struct ir_arg* cond;
   struct ir_arg* func;
   int lineno;
};

struct ir_error {
   int code;
   struct ir_func* func;
   struct ir_arg* arg;
   char* file;
   int lineno;
   int level;
};

struct ir_func {
   char* name;
   struct map params;
   struct list cases;
   char* file;
   ir_type_t type;
   int param;
   int lineno;
   int ref;
   struct ir_error err;
};

// construction
struct ir_func* ir_func_init(char* name, int lineno);
void ir_func_clear(struct ir_func* f);
struct ir_case* ir_func_case(struct ir_arg* cond, struct ir_arg* func, int lineno);
struct ir_param* ir_func_param(struct ir_func* func, char* name, int lineno);
struct ir_arg* ir_arg_op(char* func_name, struct ir_arg* a, struct ir_arg* b, int lineno);
struct ir_arg* ir_arg_word(int word, ir_type_t type, int lineno);
struct ir_arg* ir_arg_data(char* ptr, size_t size, ir_type_t type, int lineno);
struct ir_arg* ir_arg_call(struct ir_func* func, char* name, struct list* args, int lineno);

// information
ir_type_t ir_typeof(map_t* funcs, ir_type_t args, struct ir_arg* arg);

// type
ir_type_t ir_type(char* name);
bool ir_type_cmp(ir_type_t type, char* name);

// copy
struct ir_func* ir_func_cpy(struct ir_func* f);
void ir_params_cpy(struct ir_func* f, struct map params);
struct ir_arg* ir_arg_cpy(struct ir_func* f, struct ir_arg* a);
struct list ir_cases_cpy(struct ir_func* f, struct list cases);

// utils
void ir_rem_unused(map_t* funcs);

// error function
char* ir_typestr(char* buf, ir_type_t type);
bool ir_has_error(struct ir_error* err);
void ir_print_err(struct ir_error err);

#endif
