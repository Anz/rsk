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

// INFORMATION
ir_type_t ir_typeof(map_t* funcs, ir_type_t args, struct ir_arg* arg) {
   switch (arg->arg_type) {
      case IR_ARG_DATA: return arg->data.type;
      case IR_ARG_PARAM: 
         if (arg->call.param <= 0 || list_size(args) < arg->call.param)
            return NULL;
         return list_get(args, arg->call.param-1);
      case IR_ARG_CALL: {
         struct ir_func* f = map_get(funcs, arg->call.func_name, strlen(arg->call.func_name));
         if (!f) 
            return NULL;
         if (f->type)
            return f->type;
         if (f->param > 0)
            if (f->param <= list_size(args))
               return list_get(args, f->param-1);
            else
               return NULL;
         return NULL;
      }
   }
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

// UTILS
static void remove_unused_func(map_t* funcs, struct ir_func* func); 
static void decrement_ref(map_t* funcs, struct ir_arg* arg) {
   if (arg->arg_type != IR_ARG_CALL) {
      return;
   }
   char* name = arg->call.func_name;
   struct ir_func* call = map_get(funcs, name, strlen(name) + 1);
   call->ref--;
   if (call->ref == 0) {
      remove_unused_func(funcs, call);
   }
      
   for (list_it* it = list_iterator(&arg->call.args); it != NULL; it = it->next) {
      decrement_ref(funcs, it->data);
   }
}

static void remove_unused_func(map_t* funcs, struct ir_func* func) {
   map_set(funcs, func->name, strlen(func->name) + 1, NULL);

   for (list_it* it = list_iterator(&func->cases); it != NULL; it = it->next) {
      struct ir_case* c = it->data;
      struct ir_arg* arg = c->func;
      decrement_ref(funcs, arg);
   }
}

void ir_rem_unused(map_t* funcs) {
   for (map_it* it = map_iterator(funcs); it != NULL; it = it->next) {
      struct ir_func* func = it->data;
      if (func->ref != 0) {
         continue;
      }

      // remove function
      remove_unused_func(funcs, func);
   }
}

// ERROR
bool ir_has_error(struct ir_error* err) {
   return err->code > 0;
}

char* ir_typestr(char* buf, ir_type_t type) {
   if (list_size(type) == 1) {
      strcpy(buf, list_get(type, 0));
      buf += strlen(list_get(type,0));
   } else {
      *(buf++) = '(';
      for (list_it* it = list_iterator(type); it != NULL; it = it->next) {
         if (*(buf-1) != '(') {
            *(buf++) = ',';
         }

         buf = ir_typestr(buf, it->data);
      }
      *(buf++) = ')';
   }
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
         ir_typestr(a_type, a->data.type);
         ir_typestr(b_type, b->data.type);
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
