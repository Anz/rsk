#include "semantic.h"
#include <stdio.h>

static void semantic_binary_operation_check(struct ir_arg* a, struct ir_arg* b, struct list* errors) {
   struct ir_type* type_a = ir_arg_type(a);
   struct ir_type* type_b = ir_arg_type(b);
   
   if (type_a != NULL && type_b != NULL && type_a != type_b) {
      struct ir_error* error = malloc(sizeof(*error));
      memset(error, 0, sizeof(*error));
      error->msg = "on call of a + b types of a and b must be equals";
      error->lineno = a->lineno;
      list_add(errors, error);
   } else if (type_a != NULL && type_b == NULL) {
      switch (b->arg_type) {
         case IR_ARG_PARAM: b->param->type = type_a; break;
         //case IR_ARG_CALL: b->type = type_a; break;
      }
   } else if (type_a == NULL && type_b != NULL) {
      switch (a->arg_type) {
         case IR_ARG_PARAM: a->param->type = type_b; break;
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
         type.type = arg->param->type;
         return type;
      case IR_ARG_CALL: {
         if (arg->call.args.size != arg->call.func->params.l.size) {
            struct ir_error* error = malloc(sizeof(*error));
            memset(error, 0, sizeof(*error));
            error->msg = malloc(512);
            sprintf(error->msg, "calling %s: called with %i args, has %i", arg->call.func->name, arg->call.args.size, arg->call.func->params.l.size);
            error->lineno = arg->lineno;
            list_add(errors, error);
            return type;
         }
      
         for (int i = 0; i < arg->call.args.size; i++) {
            struct ir_arg* a = (struct ir_arg*) list_get(&arg->call.args, i);
            struct ir_param* p = ((struct map_entry*) list_get(&arg->call.func->params.l, i))->data;
            struct semantic_type a_type = semantic_arg_check(a, errors);
            
            if (p->type != NULL && p->type != type.type) {
               if (type.type == NULL && a->arg_type == IR_ARG_PARAM && a->param->type == NULL) {
                  a->param->type = p->type;
               }
            }
         }
         
         struct semantic_type res;
         
         struct ir_func* func = arg->call.func;
         
         char* binary_operation[] = {
            "+", "-", "*", "/"
         };
         
         
         // check if call is on binary function
         for (int i = 0; i < sizeof(binary_operation) / sizeof(char*); i++) {
            if (strcmp(binary_operation[i], func->name) == 0) {
               struct ir_arg* a = list_get(&arg->call.args, 0);
               struct ir_arg* b = list_get(&arg->call.args, 1);
               semantic_binary_operation_check(a, b, errors);
               
               type.type = ir_arg_type(a);
               type.param = 0;
               
               if (type.type != NULL) {
                  switch (i) {
                     case 0: arg->call.func = type.type->add; break;
                     case 1: arg->call.func = type.type->sub; break;
                     case 2: arg->call.func = type.type->mul; break;
                     case 3: arg->call.func = type.type->div; break;
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
