#include "ir.h"
#include <stdio.h>

// CONSTRUCTION
struct ir_func* ir_func_init(char* name, int lineno) {
   struct ir_func* f = malloc(sizeof(*f));
   memset(f, 0, sizeof(*f));
   f->name = strdup(name);
   map_init(&f->params);
   f->lineno;
   return f;
}

static void ir_arg_clear(struct ir_arg* arg) {
   switch (arg->arg_type) {
      case IR_ARG_DATA: 
         if (arg->data.size > 4) {
            free(arg->data.ptr);
         }
         break;
      case IR_ARG_CALL: {      
         for (int i = 0; i < arg->call.args.size; i++) {
            struct ir_arg* a = (struct ir_arg*) list_get(&arg->call.args, i);
            ir_arg_clear(a);
         }
         break;
      }
   }
   
   free(arg);
}

void ir_func_clear(struct ir_func* f) {
   // free name
   free(f->name);
   
   // free params
   for (map_it* it = map_iterator(&f->params); it != NULL; it = map_next(it)) {
      struct ir_param* p = it->data;
      free(p->name);
   }
   map_clear(&f->params);
   
   // free value
   if (f->value != NULL) {
      ir_arg_clear(f->value);
   }
}

struct ir_param* ir_func_param(struct ir_func* func, char* name, int lineno) {
   struct ir_param* p = malloc(sizeof(*p));
   memset(p, 0, sizeof(*p));
   p->name = strdup(name);
   p->index = map_size(&func->params);
   p->category = p->index;
   p->lineno = lineno;
   
   map_set(&func->params, p->name, strlen(p->name) + 1, p);

   return p;
}

struct ir_arg* ir_arg_word(int word, struct ir_type* type, int lineno) {
   struct ir_arg* arg = malloc(sizeof(*arg));
   memset(arg, 0, sizeof(*arg));
   arg->arg_type = IR_ARG_DATA;
   arg->data.word = word;
   arg->data.size = 4;
   arg->data.type = type;
   arg->lineno = lineno;
   
   return arg;
}

struct ir_arg* ir_arg_data(char* ptr, size_t size, struct ir_type* type, int lineno) {
   struct ir_arg* arg = malloc(sizeof(*arg));
   memset(arg, 0, sizeof(*arg));
   arg->arg_type = IR_ARG_DATA;
   arg->data.ptr = ptr;
   arg->data.size = size;
   arg->data.type = type;
   arg->lineno = lineno;
   
   return arg;
}


struct ir_arg* ir_arg_call(struct map* funcs, struct ir_func* func, char* name, struct list* args, int lineno) {
   struct ir_arg* arg = malloc(sizeof(*arg));
   memset(arg, 0, sizeof(*arg));
   arg->lineno = lineno;
   if (args != NULL) {
      arg->call.args = *args;
   }
   
   struct ir_param* param = (struct ir_param*) map_get(&func->params, name, strlen(name) + 1);
   
   // parameter
   if (param != NULL) {
      arg->arg_type = IR_ARG_PARAM;
      arg->call.param = param;
      return arg;
   }
   
   // func
   struct ir_func* call = map_get(funcs, name, strlen(name) + 1);
   if (call == NULL) {
      call = ir_func_init(name, -1);
      map_set(funcs, call->name, strlen(call->name) + 1, call);
   }
   
   arg->arg_type = IR_ARG_CALL;
   arg->call.func = call;
   
   return arg;
}

// RESOLVING
struct ir_type* ir_arg_type(struct ir_arg* arg) {
   switch (arg->arg_type) {
      case IR_ARG_DATA: return arg->data.type;
      case IR_ARG_PARAM: return arg->call.param->type;
      case IR_ARG_CALL: {
         if (arg->call.func->type != NULL) {
            return arg->call.func->type;
         }
         int param = arg->call.func->type_param;
         arg = list_get(&arg->call.args, param);
         return ir_arg_type(arg);
      }
   }
   
   return NULL;
}

struct ir_param* ir_arg_param(struct ir_arg* arg) {
   switch (arg->arg_type) {
      case IR_ARG_PARAM: return arg->call.param;
      case IR_ARG_CALL: {
         if (arg->call.func->type != NULL) {
            return NULL;
         }      
         int param = arg->call.func->type_param;
         arg = list_get(&arg->call.args, param);
         return ir_arg_param(arg);
      }
   }
   
   return NULL;
}

// ERROR
void ir_print_err(struct ir_error err) {
   struct ir_func* call;
   struct list args;
   struct ir_arg* a;
   struct ir_arg* b;
   
   if (err.arg->arg_type == IR_ARG_CALL) {
      call = err.arg->call.func;
      args = err.arg->call.args;
      
      if (args.size >= 2) {
         a = list_get(&args, 0);
         b = list_get(&args, 1);
      }
   }

   switch (err.code) {
      case IR_ERR_BIN_OP_NE: printf("line %03i operandes must of the same type: %s %s %s\n", err.lineno, a->data.type->name, call->name, a->data.type->name); break;
      case IR_ERR_NR_ARGS: printf("line %03i calling %s with %i arguments, function requires %i\n", err.lineno, call->name, args.size, call->params.l.size); break;// number of arguments wrong (too few/much)
   }
}
