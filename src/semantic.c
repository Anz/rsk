#include "semantic.h"
#include <stdio.h>

static void semantic_binary_operation_check(struct ir_arg* call, struct ir_arg* a, struct ir_arg* b, struct list* errors) {
   struct ir_type* type_a = ir_arg_type(a);
   struct ir_type* type_b = ir_arg_type(b);
   
   if (type_a != NULL && type_b != NULL && type_a != type_b) {
      struct ir_error* error = malloc(sizeof(*error));
      memset(error, 0, sizeof(*error));
      error->code = IR_ERR_BIN_OP_NE;
      error->arg = call;
      error->lineno = a->lineno;
      list_add(errors, error);
   } else if (type_a != NULL && type_b == NULL) {
      switch (b->arg_type) {
         case IR_ARG_PARAM: b->call.param->type = type_a; break;
         //case IR_ARG_CALL: b->type = type_a; break;
      }
   } else if (type_a == NULL && type_b != NULL) {
      switch (a->arg_type) {
         case IR_ARG_PARAM: a->call.param->type = type_b; break;
         //case IR_ARG_CALL: a->call.type = type_b; break;
      }
   }
}

static struct semantic_type semantic_arg_check(struct ir_arg* arg, struct list* errors) {
   struct semantic_type type;
   memset(&type, 0, sizeof(type));

   switch (arg->arg_type) {
      case IR_ARG_DATA:
         type.type = arg->data.type;
         return type;
      case IR_ARG_PARAM:
         type.type = arg->call.param->type;
         return type;
      case IR_ARG_CALL: {
         if (arg->call.args.size != arg->call.func->params.l.size) {
            struct ir_error* error = malloc(sizeof(*error));
            memset(error, 0, sizeof(*error));
            error->code = IR_ERR_NR_ARGS;
            error->arg = arg;
            error->lineno = arg->lineno;
            list_add(errors, error);
            return type;
         }
      
         for (int i = 0; i < arg->call.args.size; i++) {
            struct ir_arg* a = (struct ir_arg*) list_get(&arg->call.args, i);
            struct ir_param* p = ((struct map_entry*) list_get(&arg->call.func->params.l, i))->data;
            struct semantic_type a_type = semantic_arg_check(a, errors);
            
            if (p->type != NULL && p->type != type.type) {
               if (type.type == NULL && a->arg_type == IR_ARG_PARAM && a->call.param->type == NULL) {
                  a->call.param->type = p->type;
               }
            }
         }
         
         struct semantic_type res;
         
         struct ir_func* func = arg->call.func;
         
         char* binary_operation[] = {
            "+", "-", "*", "/", "=", "<", ">"
         };
         
         
         // check if call is on binary function
         for (int i = 0; i < sizeof(binary_operation) / sizeof(char*); i++) {
            if (strcmp(binary_operation[i], func->name) == 0) {
               struct ir_arg* a = list_get(&arg->call.args, 0);
               struct ir_arg* b = list_get(&arg->call.args, 1);
               semantic_binary_operation_check(arg, a, b, errors);
               
               type.type = ir_arg_type(a);
               type.param = 0;
               
               if (type.type != NULL) {
                  struct ir_func* op = map_get(&type.type->ops, func->name, strlen(func->name) + 1);
                  if (op != NULL) {
                     arg->call.func = op;
                  } else {
                     printf("not found %s on %s (size: %i)\n", func->name, type.type->name, map_size(&type.type->ops));
                  }
               }
               
               return type;
            }
         }
         
         res = semantic_check(arg->call.func, errors);
         
         if (res.type == NULL) {
            struct ir_arg* a = list_get(&arg->call.args, res.param);
            res.type = ir_arg_type(a);
            if (res.type == NULL) {
               res.param = ir_arg_param(a)->index;
            }
         }
         
         return res;
      }
   }
}

struct semantic_type semantic_check(struct ir_func* f, struct list* errors) {
   if (f->value == NULL) {
      struct semantic_type type;
      memset(&type, 0, sizeof(type));
      return type;
   }
   
   struct semantic_type type = semantic_arg_check(f->value, errors);
   f->type = type.type;
   f->type_param = type.param;
   return type;  
}