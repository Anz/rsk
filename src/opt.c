#include "opt.h"
#include <stdio.h>

int id = 1;

static void opt_func(map_t* funcs, struct ir_func* func, ir_type_t args); 
static void opt_arg(map_t* funcs, struct ir_arg* arg, ir_type_t args);

void optimize(map_t* funcs, char* name, ir_type_t args) {
   struct ir_func* func = map_get(funcs, name, strlen(name)+1);

   if (func == NULL) {
      printf("function not found %s\n", name);
      exit(1);
   }
   func->ref++;

   // define typeof function
   struct ir_func* f = ir_func_init("typeof", -1);
   f->ref = -1;
   map_set(funcs, f->name, strlen(f->name)+1, f);

   // remove unused functions
   ir_rem_unused(funcs);

   opt_func(funcs, func, args);
}

static void opt_func(map_t* funcs, struct ir_func* func, ir_type_t args) {
   // duplicate functoin
   for (list_it* it = list_iterator(&func->cases); it != NULL; it = it->next) {
      struct ir_case* c = it->data;
      opt_arg(funcs, c->func, args);
   }
}


static void opt_arg(map_t* funcs, struct ir_arg* arg, ir_type_t args) {
   if (arg->arg_type == IR_ARG_CALL) {
      for (list_it* it = list_iterator(&arg->call.args); it != NULL; it = it->next) {
         opt_arg(funcs, it->data, args);
      }

      if (strcmp("typeof", arg->call.func_name) == 0) {
         char* name = malloc(512);
         ir_type_t type = ir_typeof(funcs, args, list_get(&arg->call.args, 0));
         char* end = ir_typestr(name, type);
         int len = end - name;
         // TODO free memory for all sub args
         memcpy(arg, ir_arg_data(name, len, ir_type("array"), -1), sizeof(*arg));
         return;
      }

      struct ir_func* caller = map_get(funcs, arg->call.func_name, strlen(arg->call.func_name) + 1);
      struct ir_func* callee = NULL;

      if (caller->ref == 1) {
         callee = caller;
      } else {
         callee = ir_func_cpy(caller);
         caller->ref--;
         callee->ref = 1;
         char* callee_name = malloc(512);
         sprintf(callee_name, "%s%i", caller->name, id++);
         callee->name = callee_name;
         map_set(funcs, callee_name, strlen(callee_name) + 1, callee);
      }

      opt_func(funcs, callee, args); 
   }
}
