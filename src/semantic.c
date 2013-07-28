#include "semantic.h"

static struct ir_arg* semantic_arg_check(struct ir_arg* arg) {
   switch (arg->arg_type) {
      case IR_ARG_WORD: return arg;
      case IR_ARG_DATA: return arg;
      case IR_ARG_PARAM: return arg;
      case IR_ARG_CALL: {
         for (int i = 0; i < arg->call.args.size; i++) {
            struct ir_arg* a = (struct ir_arg*) list_get(&arg->call.args, i);
            semantic_arg_check(a);
         }
         
         struct ir_arg* res = NULL;
         
         struct ir_func* func = arg->call.func;
         
         if (strcmp("+", func->name) == 0) res = list_get(&arg->call.args, 0);
         else if (strcmp("-", func->name) == 0) res =  list_get(&arg->call.args, 0);
         else if (strcmp("*", func->name) == 0) res =  list_get(&arg->call.args, 0);
         else if (strcmp("/", func->name) == 0) res =  list_get(&arg->call.args, 0);
         else res = semantic_check(arg->call.func);
         
         arg->type.type = res->type.type;
         switch (res->type.type) {
            case IR_RES_UNK: break;
            case IR_RES_STA: arg->type.sta = res->type.sta; break;
            case IR_RES_DYN: arg->type.dyn = res->type.dyn; break;
         }
         // TODO replase above with:
         // memcpy(&arg.type, &res.type);
         
         return arg;
      }
      default: printf("\error %i\n", arg->type.type); break;
   }
}

struct ir_arg* semantic_check(struct ir_func* f) {
   if (f->value == NULL || f->value->type.type != IR_RES_UNK) {
      return f->value;
   }
   
   return semantic_arg_check(f->value);
}
