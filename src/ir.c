#include "ir.h"

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
