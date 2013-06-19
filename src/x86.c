#include "x86.h"
#include <stdlib.h>

struct text_ref {
   char* callee;
   int caller;
   struct text_ref* next;
};

void x86_write(char* text, size_t* index, char* data, size_t size) {
   memcpy(text+*index, data, size);
   *index += size;
}

void x86_loadp(char* text, size_t* index, int param) { 
   char code[] = { 0x8b, 0x55, (1+param)*sizeof(int), 0x52 };
   x86_write(text, index, code, sizeof(code));
}

void x86_loadv(char* text, size_t* index, int arg) {
   char code[] = { 0x68 };
   x86_write(text, index, code, sizeof(code));
   x86_write(text, index, (char*)&arg, sizeof(arg));
}

void x86_loadd(char* text, size_t* index, void* ptr, size_t size) {
   {
      char code[] = { 0x89, 0xE0, 0x50 };
      x86_write(text, index, code, sizeof(code));
   }
   for (int i = 0; i < size / 4; i++) {
      char code[] = { 0x68 };
      x86_write(text, index, code, sizeof(code));
      x86_write(text, index, ptr+i*4, 4);
      
      char cleanup[] = { 0x83, 0xC4 };
      x86_write(text, index, cleanup, sizeof(cleanup));
      char val = 4;
      x86_write(text, index, (char*)&val, sizeof(val));
   }
}

void x86_int_add(char* text, size_t* index) {
   char code[] = { 0x5A, 0x58, 0x01, 0xD0, 0x50 };
   x86_write(text, index, code, sizeof(code));
}

void x86_int_sub(char* text, size_t* index) {
   char code[] = { 0x5A, 0x58, 0x29, 0xD0, 0x50 };
   x86_write(text, index, code, sizeof(code));
}

void x86_int_mul(char* text, size_t* index) {
   char code[] = { 0x5A, 0x58, 0x0F, 0xAF, 0xc2, 0x50 };
   x86_write(text, index, code, sizeof(code));
}

void x86_int_div(char* text, size_t* index) {
   char code[] = { 0x5B, 0x58, 0x99, 0xF7, 0xFB, 0x50 };
   x86_write(text, index, code, sizeof(code));
}

void* funcs_ow[][2] = {
   { "int+", &x86_int_add },
   { "int-", &x86_int_sub },
   { "int*", &x86_int_mul },
   { "int/", &x86_int_div },
   { "float+", &x86_int_add },
};

void x86_call(struct text_ref** refs, char* text, size_t* index, struct ir_func* func) {
   // native function overwrite
   for (int i = 0; i < sizeof(funcs_ow) / sizeof(funcs_ow[0]); i++) {
      char* name = (char*)funcs_ow[i][0];
      void (*f)(char*, size_t*) = funcs_ow[i][1];
      if (strcmp(name, func->name) == 0) {
         f(text, index);
         return;
      }
   }
   
   // generic function
   char call[] = { 0xE8, 0x0, 0x0, 0x0, 0x0 };
   x86_write(text, index, call, sizeof(call));
   char cleanup[] = { 0x83, 0xC4 };
   x86_write(text, index, cleanup, sizeof(cleanup));
   char param = 0;
   for (struct ir_param* p = func->param; p != NULL; p = p->next) {
      param += sizeof(int*);
   }
   x86_write(text, index, (char*)&param, sizeof(param));
   char push[] = { 0x50 };
   x86_write(text, index, push, sizeof(push));
   
   struct text_ref* ref = malloc(sizeof(struct text_ref));
   memset(ref, 0, sizeof(ref));
   ref->callee = func->name;
   ref->caller = *index - 8;
   
   if (*refs == NULL) {
      *refs = ref;
   } else {
      ref->next = *refs;
      *refs = ref;
   }
}

void x86_func_compile(struct text_ref** refs, char* text, size_t* text_index, struct ir_arg* arg, int arg_count) {   
   switch (arg->arg_type) {
      case IR_ARG_CALL: {         
         struct ir_func* call = arg->call.func;
         
         // load params
         for (struct ir_arg* a = arg->call.arg; a != NULL; a = a->next) {
            x86_func_compile(refs, text, text_index, a, arg_count);
         }
         
         // call function
         if (strcmp("+", call->name) == 0) call = arg->type->add;
         if (strcmp("-", call->name) == 0) call = arg->type->sub;
         if (strcmp("*", call->name) == 0) call = arg->type->mul;
         if (strcmp("/", call->name) == 0) call = arg->type->div;
         x86_call(refs, text, text_index, call);
         break;
      }
      case IR_ARG_PARAM: {      
         x86_loadp(text, text_index, arg_count - arg->param->index);
         break;
      }
      case IR_ARG_WORD: {
         x86_loadv(text, text_index, arg->word);
         break;
      }
      case IR_ARG_DATA: {
         x86_loadd(text, text_index, arg->data.ptr, arg->data.size);
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
   

   nr.text = malloc(4096);
   size_t text_index = 0;
   
   nr.data = malloc(4096);
   nr.data_size = 4096;
   
   char frame_start[] = { 0x55, 0x89, 0xE5};
   char frame_stop[] = { 0x58, 0x5D, 0xC3};
   
   struct text_ref* refs = NULL;
   
   for (struct ir_func* f = funcs; f != NULL; f = f->next) {
      if (f->value == NULL) continue;
      
      symbol_t* sym = malloc(sizeof(symbol_t));
      memset(sym, 0, sizeof(sym));
      sym->name = f->name;
      sym->text = text_index;
      sym->text_size = text_index;
   
      x86_write(nr.text, &text_index, frame_start, sizeof(frame_start));
      
      int count = 0;
      for (struct ir_param* p = f->param; p != NULL; p = p->next) {
         count++;
      }
      
      x86_func_compile(&refs, nr.text, &text_index, f->value, count);
      
      x86_write(nr.text, &text_index, frame_stop, sizeof(frame_stop));
      
      sym->text_size = text_index - sym->text_size;
      
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
   }
   
   nr.text_size = text_index;
   
   return nr;
}
