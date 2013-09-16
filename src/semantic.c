#include "semantic.h"
#include "map.h"
#include <stdio.h>

struct semantic_type {
   ir_type_t type;
   int param;
};

static struct semantic_type semantic_arg_check(map_t* funcs, struct ir_arg* arg);
static void semantic_func_check(struct map* funcs, struct ir_func* func);

void semantic_check(map_t* funcs) {
   for (map_it* it = map_iterator(funcs); it != NULL; it = it->next) {
      semantic_func_check(funcs, (struct ir_func*)it->data);
   }
}

static void semantic_func_check(struct map* funcs, struct ir_func* func) {
   // check if functions has errors
   if (func->type != NULL || func->param > 0 || ir_has_error(&func->err)) {
      return;
   }

   // set return type to unknown
   func->err.code = IR_ERR_RET_TYPE_UN;

   if (func->type == NULL && func->param > 0) {
      for (int i = 0; i < list_size(&func->cases); i++) {
         struct ir_case* c = list_get(&func->cases, i);
         
         // check condition
         if (!ir_type_cmp(semantic_arg_check(funcs, c->cond).type, "bool")) {
            printf("condition must be a boolean \n");
            exit(1);
         }

         // check value
         struct semantic_type type = semantic_arg_check(funcs, c->cond);
         if (func->type == NULL && func->param == 0) {
            func->type = type.type;
            func->param = type.param;
         } else {
            if (func->type != NULL && !list_eq(func->type, type.type)) {
               printf("return type not equals\n");
               exit(1);
            } else if (func->param != type.param) {
               printf("return param not equals\n");
               exit(1);
            }
         }
      }
   }
}

static struct semantic_type semantic_arg_check(map_t* funcs, struct ir_arg* arg) {
   struct semantic_type type;
   memset(&type, 0, sizeof(type));

   switch (arg->arg_type) {
      case IR_ARG_DATA: 
         type.type = arg->data.type;
         break;
         
      case IR_ARG_PARAM: {
         type.param = arg->call.param + 1;
         break;
      }
      case IR_ARG_CALL: {
         printf("calling %s\n", arg->call.func_name);
         struct ir_func* call = map_get(funcs, arg->call.func_name, strlen(arg->call.func_name) + 1);
         semantic_func_check(funcs, call);
         type.type = call->type;
         type.param = call->param;
         break;
      }
   }

   return type;
}
