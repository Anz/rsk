#include "semantic.h"
#include <stdio.h>

static void concat(char* dst, char delimitor, int n, char* strs[]) {
   for (int i = 0; i <n; i++) {
      char* str = strs[i];
      strcpy(dst, str);
      if (i < n - 1) {
         dst += strlen(str);
         *(dst++) = delimitor;
      }
   }
}

static struct ir_type* semantic_arg_check(struct map* funcs, struct ir_arg* arg, struct list args, struct list* errors) {   
   if (arg == NULL) {
      return NULL;
   }

   switch (arg->arg_type) {
      case IR_ARG_DATA: return arg->data.type;
      case IR_ARG_PARAM: return list_get(&args, arg->call.param->index);
      case IR_ARG_CALL: {
         if (arg->call.args.size != arg->call.func->params.l.size) {
            struct ir_error* error = malloc(sizeof(*error));
            memset(error, 0, sizeof(*error));
            error->code = IR_ERR_NR_ARGS;
            error->arg = arg;
            error->lineno = arg->lineno;
            list_add(errors, error);
            return NULL;
         }
         struct list arg_types;
         list_init(&arg_types);
         
         for (int i = 0; i < arg->call.args.size; i++) {
            struct ir_arg* a = (struct ir_arg*) list_get(&arg->call.args, i);
            
            list_add(&arg_types, semantic_arg_check(funcs, a, args, errors));
         }
         
         struct ir_func* func = arg->call.func;
         
         
         char* binary_operation[] = {
            "+", "-", "*", "/", "=", "!=", "<=", ">=", "<", ">", ""
         };
         // check if call is on binary function
         for (int i = 0; binary_operation[i] != ""; i++) {
            if (strcmp(binary_operation[i], func->name) == 0) {
               struct ir_type* type = list_get(&arg_types, 0);
               if (type == NULL) {
                  return NULL;
               }
               arg->call.func = map_get(&type->ops, func->name, strlen(func->name)+1);
               return type;
            }
         }
         
         arg->call.func = semantic_check(funcs, arg->call.func, arg_types, errors);
         return arg->call.func->type;
      }
   }
}

struct ir_func* semantic_check(struct map* funcs, struct ir_func* f, struct list args, struct list* errors) {
   {
      struct ir_func* found = map_get(funcs, f->name, strlen(f->name)+1);
      if (found == NULL) {
         printf("function not found %s\n", f->name);
         exit(1);
      }
   }

   if (list_size(&f->cases) == 0) {
      return f;
   }
   
   int num = list_size(&args);
   
   char* strs[num+1];
   strs[0] = f->name;
   for (int i = 0; i < num; i++) {
      struct ir_type* type = list_get(&args, i);
      strs[i+1] = type->name;
   }
   
   char* name = malloc(512);
   concat(name, '_', num+1, strs);
   
   struct ir_func* func_new = NULL;
   
   
   func_new = map_get(funcs, name, strlen(name)+1);
   if (func_new != NULL) {
      if ( (num == 0 && func_new->type != NULL) || (num > 0) ) {
         return func_new;
      }
   } else {  
      func_new = ir_func_cpy(f);
      func_new->ref = 1;
      f->ref--;
      func_new->name = name;
      map_set(funcs, name, strlen(name)+1, func_new);
   }
   
   int j;
   for (int i = 0; i < list_size(&func_new->cases); i++) {
      struct ir_case* c = list_get(&func_new->cases, i);
       
      semantic_arg_check(funcs, c->cond, args, errors);
      struct ir_type* type = semantic_arg_check(funcs, c->func, args, errors);
      
      if (type != NULL) {
         j = i;
         func_new->type = type;
         break;
      }
   }
   
   
   if (func_new->type == NULL) {
      printf("infinity loop in %s\n", f->name);
      exit(1);
   }
   
   for (int i = 0; i < list_size(&func_new->cases); i++) {
      if (j == i) {
         continue;
      }
      struct ir_case* c = list_get(&func_new->cases, i);
      
      semantic_arg_check(funcs, c->cond, args, errors);
      struct ir_type* type = semantic_arg_check(funcs, c->func, args, errors);
      
      if (type == NULL) {
         printf("cases %i in %s does not have an explicit return type\n", i+1, func_new->name);
         exit(1);
      }
      
      if (type != func_new->type) {
         printf("cases in %s do not have the same return type (%p != %p)\n", func_new->name, type, func_new->type);
         exit(1);
      }
   }
   
   return func_new;
}
