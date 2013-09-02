#include "semantic.h"
#include "map.h"
#include <stdio.h>

static struct semantic_type semantic_func_check(struct ir_func* func, map_t* infos);
static struct semantic_type semantic_arg_check(struct ir_arg* arg, map_t* infos);
static struct info* info_create(struct ir_arg* arg, map_t* infos);

void semantic_check(struct map* funcs, map_t* infos) {
   // iterate over all funcs
   for (map_it* it = map_iterator(funcs); it != NULL; it = it->next) {
      semantic_func_check(it->data, infos);
   }
}

static struct semantic_type semantic_func_check(struct ir_func* func, map_t* infos) {
   if (list_size(&func->cases) == 0) {
      struct semantic_type type;
      memset(&type, 0, sizeof(type));
      return type;
   }
   
   struct info* info = map_get(infos, func->name, strlen(func->name) + 1);
   if (info != NULL) {
      return info->type;
   }
   

   // function information
   info = malloc(sizeof(*info));
   memset(info, 0, sizeof(*info));
   info->func = func;
   map_set(infos, func->name, strlen(func->name)+1, info);
   
   struct ir_case* c = list_get(&func->cases, 0);
   info->type = semantic_arg_check(c->func, infos);
   
   int j;
   for (int i = 0; i < list_size(&func->cases); i++) {
      struct ir_case* c = list_get(&func->cases, i);
      
      info->type = semantic_arg_check(c->func, infos);
      
      if (info->type.type != NULL || info->type.param > 0) {
         j = i;
         break;
      }
   }
   
   // check retun type
   if (ir_has_error(&info->type.error)) {
      info->error = info->type.error;
      info->error.func = func;
      info->error.file = func->file;
      info->error.lineno = func->lineno;
      info->error.level = IR_LVL_SUBSEQ;
      return info->type;
   } else if (info->type.type == NULL && info->type.param <= 0) {
      info->error.code = IR_ERR_RET_TYPE_UN;
      info->error.func = func;
      info->error.file = func->file;
      info->error.lineno = func->lineno;
      info->error.level = IR_LVL_SOURCE;
      info->type.error = info->error;
      return info->type;
   }
   
   for (int i = 0; i < list_size(&func->cases); i++) {
      if (j == i) {
         continue;
      }
      struct ir_case* c = list_get(&func->cases, i);
      
      // check condition
      struct semantic_type cond_type = semantic_arg_check(c->cond, infos);
      if (c->cond != NULL && ( (cond_type.type != NULL && strcmp(cond_type.type->name, "bool") != 0) || (cond_type.param > 0) )) { // TODO valid if param is bool
         printf("type: %p\n", cond_type.type);
         info->error.code = IR_ERR_COND_TYPE;
         info->error.func = func;
         info->error.file = func->file;
         info->error.lineno = c->lineno;
         info->error.level = IR_LVL_SOURCE;
         info->type.error = info->error;
         return info->type;
      }
      
      struct semantic_type type = semantic_arg_check(c->func, infos);
      
      // check return type
      if (info->type.type != type.type || info->type.param != type.param) {
         info->error.code = IR_ERR_RET_TYPE_NE;
         info->error.func = func;
         info->error.file = func->file;
         info->error.lineno = func->lineno;
         info->error.level = IR_LVL_SOURCE;
         info->type.error = info->error;
         return info->type;
      }
   }
   
   return info->type;
}

static struct semantic_type semantic_arg_check(struct ir_arg* arg, map_t* infos) {
   struct semantic_type type;
   memset(&type, 0, sizeof(type));

   switch (arg->arg_type) {
      case IR_ARG_DATA: 
         type.type = arg->data.type;
         return type;
         
      case IR_ARG_PARAM:
         type.param = arg->call.param->index + 1;
         return type;
         
      case IR_ARG_CALL:
         return semantic_func_check(arg->call.func, infos);
   }
}
