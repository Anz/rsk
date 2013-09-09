#include "ir.h"
#include <stdio.h>

// CONSTRUCTION
struct ir_func* ir_func_init(char* name, int lineno) {
   struct ir_func* f = malloc(sizeof(*f));
   memset(f, 0, sizeof(*f));
   f->name = strdup(name);
   map_init(&f->params);
   list_init(&f->cases);
   f->lineno = lineno;
   return f;
}

static void ir_arg_clear(struct ir_arg* arg) {
   if (arg == NULL) {
      return;
   }

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
   for (int i = 0; i < list_size(&f->cases); i++) {
      struct ir_case* c = list_get(&f->cases, i);
      ir_arg_clear(c->cond);
      ir_arg_clear(c->func);
   }
}

struct ir_case* ir_func_case(struct ir_arg* cond, struct ir_arg* func, int lineno) {
   struct ir_case* c = malloc(sizeof(*c));
   memset(c, 0, sizeof(*c));
   c->cond = cond;
   c->func = func;
   c->lineno = lineno;
}

struct ir_param* ir_func_param(struct ir_func* func, char* name, int lineno) {
   struct ir_param* p = malloc(sizeof(*p));
   memset(p, 0, sizeof(*p));
   p->name = strdup(name);
   p->index = map_size(&func->params);
   p->lineno = lineno;
   
   map_set(&func->params, p->name, strlen(p->name) + 1, p);

   return p;
}

struct ir_arg* ir_arg_op(char* func_name, struct ir_arg* a, struct ir_arg* b, int lineno) {
   struct ir_arg* arg = malloc(sizeof(*arg));
   memset(arg, 0, sizeof(*arg));
   arg->arg_type = IR_ARG_CALL;
   arg->call.func_name = func_name;
   list_init(&arg->call.args);
   list_add(&arg->call.args, a);
   list_add(&arg->call.args, b);
   arg->lineno = lineno;
   
   return arg;
}

struct ir_arg* ir_arg_word(int word, ir_type_t type, int lineno) {
   struct ir_arg* arg = malloc(sizeof(*arg));
   memset(arg, 0, sizeof(*arg));
   arg->arg_type = IR_ARG_DATA;
   arg->data.word = word;
   arg->data.size = 4;
   arg->data.type = type;
   arg->lineno = lineno;
   
   return arg;
}

struct ir_arg* ir_arg_data(char* ptr, size_t size, ir_type_t type, int lineno) {
   struct ir_arg* arg = malloc(sizeof(*arg));
   memset(arg, 0, sizeof(*arg));
   arg->arg_type = IR_ARG_DATA;
   arg->data.ptr = ptr;
   arg->data.size = size;
   arg->data.type = type;
   arg->lineno = lineno;
   
   return arg;
}


struct ir_arg* ir_arg_call(struct ir_func* func, char* name, struct list* args, int lineno) {
   func->ref++;

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
      arg->call.param = param->index;
      return arg;
   }
   
   // func
   arg->arg_type = IR_ARG_CALL;
   arg->call.func_name = name;
   
   return arg;
}

// TYPE
ir_type_t ir_type(char* name) {
   ir_type_t type = malloc(sizeof(type));
   list_init(type);
   list_add(type, name);
   return type;
}

bool ir_type_cmp(ir_type_t type, char* name) {
   if (list_size(type) != 1) {
      return false;
   }

   return strcmp(list_get(type, 0), name) == 0;
}

// COPY
struct ir_func* ir_func_cpy(struct ir_func* f) {
   struct ir_func* cpy = ir_func_init(f->name, f->lineno);
   ir_params_cpy(cpy, f->params);
   cpy->cases = ir_cases_cpy(cpy, f->cases);
   return cpy;
}

void ir_params_cpy(struct ir_func* f, struct map params) {   
   for (map_it* it = map_iterator(&params); it != NULL; it = it->next) {
      struct ir_param* p = it->data;
      struct ir_param* param = ir_func_param(f, p->name, p->lineno);
   }
}

struct ir_arg* ir_arg_cpy(struct ir_func* f, struct ir_arg* a) {
   if (a == NULL) {
      return NULL;
   }

   switch (a->arg_type) {
      case IR_ARG_CALL: {
         struct ir_arg* arg = malloc(sizeof(*arg));
         memset(arg, 0, sizeof(*arg));
         arg->arg_type = IR_ARG_CALL;
         arg->call.func_name = a->call.func_name;
         arg->lineno = a->lineno;
         for (list_it* it = list_iterator(&a->call.args); it != NULL; it = it->next) {
            list_add(&arg->call.args, ir_arg_cpy(f, it->data));
         }
         return arg;
      }
      case IR_ARG_PARAM: {
         struct ir_arg* arg = malloc(sizeof(*arg));
         memset(arg, 0, sizeof(*arg));
         arg->arg_type = IR_ARG_PARAM;
         arg->call.param = a->call.param;
         arg->lineno = a->lineno;
         for (list_it* it = list_iterator(&a->call.args); it != NULL; it = it->next) {
            list_add(&arg->call.args, ir_arg_cpy(f, it->data));
         }
         return arg;
      }
      case IR_ARG_DATA:
         return ir_arg_data(a->data.ptr, a->data.size, a->data.type, a->lineno);
   }
}

struct list ir_cases_cpy(struct ir_func* f, struct list cases) {
   struct list cpy;
   list_init(&cpy);
   for (list_it* it = list_iterator(&cases); it != NULL; it = it->next) {
      struct ir_case* c = it->data;
      list_add(&cpy, ir_func_case(ir_arg_cpy(f, c->cond), ir_arg_cpy(f, c->func), c->lineno));
   }
   return cpy;
}

// ERROR
bool ir_has_error(struct ir_error* err) {
   return err->code > 0;
}

static char* typestr(char* buf, ir_type_t type) {
   *(buf++) = '(';
   for (int j = 0; j < list_size(type); j++) {
      if (j > 0) {
         *(buf++) = ',';
      }

      ir_type_t t = list_get(type, j);
      if (list_size(t) == 1) {
         buf = strcpy(buf, list_get(t, 0)) + 1;
      } else {
         buf = typestr(buf, t);
      }
   }
   *(buf++) = ')';
   return buf;
}

void ir_print_err(struct ir_error err) {
   char* call;
   struct list args;
   struct ir_arg* a;
   struct ir_arg* b;
   
   if (err.arg != NULL && err.arg->arg_type == IR_ARG_CALL) {
      call = err.arg->call.func_name;
      args = err.arg->call.args;
      
      if (args.size >= 2) {
         a = list_get(&args, 0);
         b = list_get(&args, 1);
      }
   }
   
   fprintf(stderr, "%s:%i\t", err.file, err.lineno);

   switch (err.code) {
      case IR_ERR_BIN_OP_NE: { 
         char a_type[512], b_type[512];
         typestr(a_type, a->data.type);
         typestr(b_type, b->data.type);
         fprintf(stderr, "operandes must of the same type: %s %s %s\n", a_type, call, b_type);
         break;
      }
      case IR_ERR_NR_ARGS: 
         fprintf(stderr, "calling %s with %i arguments, function requires --\n", call, args.size);
         break;
      case IR_ERR_RET_TYPE_UN:
         fprintf(stderr, "return type of '%s' is unknown\n",  err.func->name);
         break;
      case IR_ERR_RET_TYPE_NE:
         fprintf(stderr, "return type of '%s' is not equal in all cases\n",  err.func->name);
         break;
      case IR_ERR_COND_TYPE:
         fprintf(stderr, "return type in condition in function '%s' must alway be a boolean\n",  err.func->name);
         break;
   }
}
