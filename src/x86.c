#include "x86.h"
#include <stdlib.h>
#include <stdbool.h>

struct text_ref {
   char* callee;
   int caller;
   struct text_ref* next;
};

void x86_func_compile(struct text_ref** refs, struct nr* nr, struct ir_arg arg, int arg_count);

static void x86_load_param(struct text_ref** refs, struct nr* nr, struct list args, int arg_count) {
   for (int i = args.size - 1; i >= 0; i--) {
      struct ir_arg arg =  *(struct ir_arg*) list_get(&args, i);
      x86_func_compile(refs, nr, arg, arg_count);
   }
}

void x86_loadp(struct nr* nr, int param) { 
   char code[] = { 0x8b, 0x55, (1+param)*sizeof(int), 0x52 };
   buffer_write(&nr->text, code, sizeof(code));
}

void x86_loadv(struct nr* nr, int arg, bool write) {
   char code[] = { 0x68 };
   buffer_write(&nr->text, code, sizeof(code));
   buffer_writew(&nr->text, arg);
   
   if (write) {
      char print[] = {      
         0x59,                          // pop    %ecx
         0x8b, 0x11,                    // mov    (%ecx),%edx
         0x83, 0xc1, 0x04,              // add    $0x4,%ecx
         0xbb, 0x01, 0x00, 0x00, 0x00,  // mov    $0x1,%ebx
         0xb8, 0x04, 0x00, 0x00, 0x00,  // mov   $0x4,%eax
         0xcd, 0x80,               	    // int    $0x80
      };
      buffer_write(&nr->text, print, sizeof(print));
   }
}

void x86_loadd(struct nr* nr, void* ptr) {
      char load[] = { 0xA1 };
      buffer_write(&nr->text, load, sizeof(load));
      buffer_write(&nr->text, &ptr, sizeof(&ptr));
      char push[] = { 0x50 };
      buffer_write(&nr->text, push, sizeof(push));
}

void x86_int_add(struct text_ref** refs, struct nr* nr, struct ir_arg arg, int arg_count) {
   x86_load_param(refs, nr, arg.call.args, arg_count);
   
   char code[] = { 0x5A, 0x58, 0x01, 0xD0, 0x50 };
   buffer_write(&nr->text, code, sizeof(code));
}

void x86_int_sub(struct text_ref** refs, struct nr* nr, struct ir_arg arg, int arg_count) {
   x86_load_param(refs, nr, arg.call.args, arg_count);
   
   char code[] = { 0x5A, 0x58, 0x29, 0xD0, 0x50 };
   buffer_write(&nr->text, code, sizeof(code));
}

void x86_int_mul(struct text_ref** refs, struct nr* nr, struct ir_arg arg, int arg_count) {
   x86_load_param(refs, nr, arg.call.args, arg_count);
   
   char code[] = { 0x5A, 0x58, 0x0F, 0xAF, 0xc2, 0x50 };
   buffer_write(&nr->text, code, sizeof(code));
}

void x86_int_div(struct text_ref** refs, struct nr* nr, struct ir_arg arg, int arg_count) {
   x86_load_param(refs, nr, arg.call.args, arg_count);
   
   char code[] = { 0x5B, 0x58, 0x99, 0xF7, 0xFB, 0x50 };
   buffer_write(&nr->text, code, sizeof(code));
}

void x86_array_add(struct text_ref** refs, struct nr* nr, struct ir_arg arg, int arg_count) {
   struct list args = arg.call.args;
   
   for (int i = 0; i < args.size; i++) {
      struct ir_arg* arg =  (struct ir_arg*) list_get(&args, i);
      x86_func_compile(refs, nr, *arg, arg_count);
   }
}

void* funcs_ow[][2] = {
   { "int+", &x86_int_add },
   { "int-", &x86_int_sub },
   { "int*", &x86_int_mul },
   { "int/", &x86_int_div },
   { "float+", &x86_int_add },
   { "array+", &x86_array_add },
};

void x86_call(struct text_ref** refs, struct nr* nr, struct ir_arg arg, int arg_count) {
   struct ir_func* func = arg.call.func;
   
   // call function
   if (strcmp("+", func->name) == 0) func = arg.res_type->add;
   if (strcmp("-", func->name) == 0) func = arg.res_type->sub;
   if (strcmp("*", func->name) == 0) func = arg.res_type->mul;
   if (strcmp("/", func->name) == 0) func = arg.res_type->div;

   // native function overwrite
   for (int i = 0; i < sizeof(funcs_ow) / sizeof(funcs_ow[0]); i++) {
      char* name = (char*)funcs_ow[i][0];
      void (*f)(struct text_ref**, struct nr*,  struct ir_arg, int) = funcs_ow[i][1];
      if (strcmp(name, func->name) == 0) {
         f(refs, nr, arg, arg_count);
         return;
      }
   }
   
   x86_load_param(refs, nr, arg.call.args, arg_count);
   
   // generic function
   char call[] = { 0xE8, 0x0, 0x0, 0x0, 0x0 };
   buffer_write(&nr->text, call, sizeof(call));
   char cleanup[] = { 0x83, 0xC4 };
   buffer_write(&nr->text, cleanup, sizeof(cleanup));
   char param = sizeof(int*) * func->params.l.size;
   buffer_write(&nr->text, (char*)&param, sizeof(param));
   char push[] = { 0x90 }; // TODO: delete NOP = 0x90
   buffer_write(&nr->text, push, sizeof(push));
   
   struct text_ref* ref = malloc(sizeof(struct text_ref));
   memset(ref, 0, sizeof(ref));
   ref->callee = func->name;
   ref->caller = nr->text.size - 8;
   
   if (*refs == NULL) {
      *refs = ref;
   } else {
      ref->next = *refs;
      *refs = ref;
   }
}

void x86_func_compile(struct text_ref** refs, struct nr* nr, struct ir_arg arg, int arg_count) {
   switch (arg.arg_type) {
      case IR_ARG_CALL: {         
         x86_call(refs, nr, arg, arg_count);
         break;
      }
      case IR_ARG_PARAM: {
         x86_loadp(nr, arg_count - arg.param);
         break;
      }
      case IR_ARG_WORD: {
         x86_loadv(nr, arg.word, false);
         break;
      }
      case IR_ARG_DATA: {
         x86_loadv(nr, 0x08048274+nr->data.size, true);
         buffer_writew(&nr->data, arg.data.size);
         buffer_write(&nr->data, arg.data.ptr, arg.data.size);
         break;
      }
      default:
         break;
   }
}


struct nr x86_compile(struct ir_func* funcs) {
   struct nr nr;
   memset(&nr, 0, sizeof(nr));
   symbol_t* last = NULL;
   
   buffer_init(&nr.text, 512);
   buffer_init(&nr.data, 512);
   
   char frame_start[] = { 0x55, 0x89, 0xE5};
   char frame_stop[] = { 0x5D, 0xC3};
   
   struct text_ref* refs = NULL;
   
   {  
      struct ir_func _print;
      memset(&_print, 0, sizeof(_print));
      _print.name = "main";
      
      struct ir_arg _stdin;
      _stdin.arg_type = IR_ARG_WORD;
      _stdin.word = 5;
      
      struct ir_arg _call;
      memset(&_call, 0, sizeof(_call));
      _call.arg_type = IR_ARG_CALL;
      _call.call.func = &_print;
      list_add(&_call.call.args, &_stdin);
      
      x86_call(&refs, &nr, _call, 0);
      
      // end
      char code[] = {
         0xbb, 0x00, 0x00, 0x00, 0x00,  // mov    $0x0,%ebx
         0xb8, 0x01, 0x00, 0x00, 0x00,  // mov    $0x1,%eax
         0xcd, 0x80,                	 // int    $0x80
      };
      buffer_write(&nr.text, code, sizeof(code));
   }
   
   for (struct ir_func* f = funcs; f != NULL; f = f->next) {
      if (f->value.arg_type == 0) {
         continue;
      }
   
      symbol_t* sym = malloc(sizeof(symbol_t));
      memset(sym, 0, sizeof(sym));
      sym->name = f->name;
      sym->text = nr.text.size;
      sym->text_size = nr.text.size;
   
      buffer_write(&nr.text, frame_start, sizeof(frame_start));
      
      int count = f->params.l.size;
      
      x86_func_compile(&refs, &nr, f->value, count);
      
      buffer_write(&nr.text, frame_stop, sizeof(frame_stop));
      
      sym->text_size = nr.text.size - sym->text_size;
      
      if (nr.symbols == NULL) {
         nr.symbols = sym;
         last = sym;
      } else {
         last->next = sym;
         last = sym;
      }
   }
   
   for (struct text_ref* r = refs; r != NULL; r = r->next) {
      symbol_t* sym = NULL;
      for (symbol_t* s = nr.symbols; s != NULL; s = s->next) {
         if (strcmp(s->name, r->callee) == 0) {
            sym = s;
            break;
         }
      }
      
      if (sym == NULL) {
         printf("x86 error did not found function: %s\n", r->callee);
         exit(1);
      }
      
      int addr = sym->text - r->caller - 4;
      memcpy((char*)(nr.text.data+r->caller), &addr, sizeof(sym->text));
   };
   nr.text.size = 512;
   return nr;
}
