#include "ir.h"
#include <stdio.h>

struct ir_type* ir_arg_type(struct ir_arg* arg) {
   switch (arg->arg_type) {
      case IR_ARG_DATA: return arg->data.type;
      case IR_ARG_PARAM: return arg->param->type;
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
      case IR_ARG_PARAM: return arg->param;
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
