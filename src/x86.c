#include "x86.h"
#include <stdlib.h>

struct text_ref {
   char* callee;
   int caller;
   struct text_ref* next;
};

void x86_func_compile(struct text_ref** refs, struct nr* nr, struct ir_arg* arg, int arg_count);

static void x86_load_param(struct text_ref** refs, struct nr* nr, struct ir_arg* arg, int arg_count) {
   if (arg == NULL) {
      return;
   }
   x86_load_param(refs, nr, arg->next, arg_count);
   x86_func_compile(refs, nr, arg, arg_count);
}

void x86_write(struct nr* nr, void* data, size_t size) {
   memcpy(nr->text+nr->text_size, data, size);
   nr->text_size += size;
}

void x86_loadp(struct nr* nr, int param) { 
   char code[] = { 0x8b, 0x55, (1+param)*sizeof(int), 0x52 };
   x86_write(nr, code, sizeof(code));
   
   char print[] = {      
      0x59,                          // pop    %ecx
      0x8b, 0x11,                    // mov    (%ecx),%edx
      0x83, 0xc1, 0x04,              // add    $0x4,%ecx
      0xbb, 0x01, 0x00, 0x00, 0x00,  // mov    $0x1,%ebx
      0xb8, 0x04, 0x00, 0x00, 0x00,  // mov   $0x4,%eax
      0xcd, 0x80,               	    // int    $0x80
   };
   x86_write(nr, print, sizeof(print));
}

void x86_loadv(struct nr* nr, int arg) {
   char code[] = { 0x68 };
   x86_write(nr, code, sizeof(code));
   x86_write(nr, (char*)&arg, sizeof(arg));
   
   char print[] = {      
      0x59,                          // pop    %ecx
      0x8b, 0x11,                    // mov    (%ecx),%edx
      0x83, 0xc1, 0x04,              // add    $0x4,%ecx
      0xbb, 0x01, 0x00, 0x00, 0x00,  // mov    $0x1,%ebx
      0xb8, 0x04, 0x00, 0x00, 0x00,  // mov   $0x4,%eax
      0xcd, 0x80,               	    // int    $0x80
   };
   x86_write(nr, print, sizeof(print));
}

void x86_loadd(struct nr* nr, void* ptr) {
      char load[] = { 0xA1 };
      x86_write(nr, load, sizeof(load));
      x86_write(nr, &ptr, sizeof(&ptr));
      char push[] = { 0x50 };
      x86_write(nr, push, sizeof(push));
}

void x86_int_add(struct text_ref** refs, struct nr* nr, struct ir_arg* arg, int arg_count) {
   x86_load_param(refs, nr, arg->call.arg, arg_count);
   
   char code[] = { 0x5A, 0x58, 0x01, 0xD0, 0x50 };
   x86_write(nr, code, sizeof(code));
}

void x86_int_sub(struct text_ref** refs, struct nr* nr, struct ir_arg* arg, int arg_count) {
   x86_load_param(refs, nr, arg->call.arg, arg_count);
   
   char code[] = { 0x5A, 0x58, 0x29, 0xD0, 0x50 };
   x86_write(nr, code, sizeof(code));
}

void x86_int_mul(struct text_ref** refs, struct nr* nr, struct ir_arg* arg, int arg_count) {
   x86_load_param(refs, nr, arg->call.arg, arg_count);
   
   char code[] = { 0x5A, 0x58, 0x0F, 0xAF, 0xc2, 0x50 };
   x86_write(nr, code, sizeof(code));
}

void x86_int_div(struct text_ref** refs, struct nr* nr, struct ir_arg* arg, int arg_count) {
   x86_load_param(refs, nr, arg->call.arg, arg_count);
   
   char code[] = { 0x5B, 0x58, 0x99, 0xF7, 0xFB, 0x50 };
   x86_write(nr, code, sizeof(code));
}

void x86_array_add(struct text_ref** refs, struct nr* nr, struct ir_arg* arg, int arg_count) {
   //x86_load_param(refs, nr, arg->call.arg, arg_count);
   
   for (struct ir_arg* a = arg->call.arg; a != NULL; a = a->next) {
      x86_func_compile(refs, nr, a, arg_count);
   
      /*if (a->arg_type != IR_ARG_CALL) {
         char code[] = {      
            0x59,                          // pop    %ecx
            0x8b, 0x11,                    // mov    (%ecx),%edx
            0x83, 0xc1, 0x04,              // add    $0x4,%ecx
            0xbb, 0x01, 0x00, 0x00, 0x00,  // mov    $0x1,%ebx
            0xb8, 0x04, 0x00, 0x00, 0x00,  // mov   $0x4,%eax
            0xcd, 0x80,               	    // int    $0x80
         };
         x86_write(nr, code, sizeof(code));
      } else {
         char code[] = {      
            0x59,                          // pop    %ecx
         };
         x86_write(nr, code, sizeof(code));
      }*/
   }
   
   /*char code[] = {
      0x6a, 0x00,                    // push 0m 
   };
   x86_write(nr, code, sizeof(code));*/
}

void* funcs_ow[][2] = {
   { "int+", &x86_int_add },
   { "int-", &x86_int_sub },
   { "int*", &x86_int_mul },
   { "int/", &x86_int_div },
   { "float+", &x86_int_add },
   { "array+", &x86_array_add },
};

void x86_call(struct text_ref** refs, struct nr* nr, struct ir_arg* arg, int arg_count) {
   struct ir_func* func = arg->call.func;
   
   // call function
   if (strcmp("+", func->name) == 0) func = arg->type->add;
   if (strcmp("-", func->name) == 0) func = arg->type->sub;
   if (strcmp("*", func->name) == 0) func = arg->type->mul;
   if (strcmp("/", func->name) == 0) func = arg->type->div;

   // native function overwrite
   for (int i = 0; i < sizeof(funcs_ow) / sizeof(funcs_ow[0]); i++) {
      char* name = (char*)funcs_ow[i][0];
      void (*f)(struct text_ref** refs, struct nr* nr, struct ir_arg* arg, int arg_count) = funcs_ow[i][1];
      if (strcmp(name, func->name) == 0) {
         f(refs, nr, arg, arg_count);
         return;
      }
   }
   
   x86_load_param(refs, nr, arg->call.arg, arg_count);
   
   // generic function
   char call[] = { 0xE8, 0x0, 0x0, 0x0, 0x0 };
   x86_write(nr, call, sizeof(call));
   char cleanup[] = { 0x83, 0xC4 };
   x86_write(nr, cleanup, sizeof(cleanup));
   char param = 0;
   for (struct ir_param* p = func->param; p != NULL; p = p->next) {
      param += sizeof(int*);
   }
   x86_write(nr, (char*)&param, sizeof(param));
   char push[] = { 0x50 };
   x86_write(nr, push, sizeof(push));
   
   struct text_ref* ref = malloc(sizeof(struct text_ref));
   memset(ref, 0, sizeof(ref));
   ref->callee = func->name;
   ref->caller = nr->text_size - 8;
   
   if (*refs == NULL) {
      *refs = ref;
   } else {
      ref->next = *refs;
      *refs = ref;
   }
}

void x86_func_compile(struct text_ref** refs, struct nr* nr, struct ir_arg* arg, int arg_count) {   
   switch (arg->arg_type) {
      case IR_ARG_CALL: {         
         x86_call(refs, nr, arg, arg_count);
         break;
      }
      case IR_ARG_PARAM: {      
         x86_loadp(nr, arg_count - arg->param->index);
         break;
      }
      case IR_ARG_WORD: {
         x86_loadv(nr, arg->word);
         break;
      }
      case IR_ARG_DATA: {
         x86_loadv(nr, 0x08048274+nr->data.size);
         buffer_writew(&nr->data, arg->data.size);
         buffer_write(&nr->data, arg->data.ptr, arg->data.size);
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
   
   nr.text = malloc(512);
   buffer_init(&nr.data, 512);
   
   char frame_start[] = { 0x55, 0x89, 0xE5};
   //char frame_stop[] = { 0x58, 0x5D, 0xC3};
   char frame_stop[] = { 0x5D, 0xC3};
   
   struct text_ref* refs = NULL;
   
   {  
      struct ir_func _print;
      memset(&_print, 0, sizeof(_print));
      _print.name = "main";
      
      struct ir_arg _call;
      memset(&_call, 0, sizeof(_call));
      _call.arg_type = IR_ARG_CALL;
      _call.call.func = &_print;
      
      x86_call(&refs, &nr, &_call, 0);
      
      // end
      char code[] = {
         0xbb, 0x00, 0x00, 0x00, 0x00,  // mov    $0x0,%ebx
         0xb8, 0x01, 0x00, 0x00, 0x00,  // mov    $0x1,%eax
         0xcd, 0x80,                	 // int    $0x80
      };
      x86_write(&nr, code, sizeof(code));
   }
   
   for (struct ir_func* f = funcs; f != NULL; f = f->next) {
      if (f->value == NULL) continue;
      
      symbol_t* sym = malloc(sizeof(symbol_t));
      memset(sym, 0, sizeof(sym));
      sym->name = f->name;
      sym->text = nr.text_size;
      sym->text_size = nr.text_size;
   
      x86_write(&nr, frame_start, sizeof(frame_start));
      
      int count = 0;
      for (struct ir_param* p = f->param; p != NULL; p = p->next) {
         count++;
      }
      
      x86_func_compile(&refs, &nr, f->value, count);
      
      x86_write(&nr, frame_stop, sizeof(frame_stop));
      
      sym->text_size = nr.text_size - sym->text_size;
      
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
      memcpy((char*)(nr.text+r->caller), &addr, sizeof(sym->text));
   };
   nr.text_size = 512;
   
   return nr;
}