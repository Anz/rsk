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
   char code[] = { 0x8b, 0x55, (2+param)*sizeof(int), 0x52 };
   x86_write(text, index, code, sizeof(code));
}

void x86_loadv(char* text, size_t* index, int arg) {
   char code[] = { 0x68 };
   x86_write(text, index, code, sizeof(code));
   x86_write(text, index, (char*)&arg, sizeof(arg));
}

void x86_add(char* text, size_t* index) {
   char code[] = { 0x5A, 0x58, 0x01, 0xD0, 0x50 };
   x86_write(text, index, code, sizeof(code));
}

void x86_sub(char* text, size_t* index) {
   char code[] = { 0x5A, 0x58, 0x29, 0xD0, 0x50 };
   x86_write(text, index, code, sizeof(code));
}

void x86_mul(char* text, size_t* index) {
   char code[] = { 0x5A, 0x58, 0x0F, 0xAF, 0xc2, 0x50 };
   x86_write(text, index, code, sizeof(code));
}

void x86_div(char* text, size_t* index) {
   char code[] = { 0x5B, 0x58, 0x99, 0xF7, 0xFB, 0x50 };
   x86_write(text, index, code, sizeof(code));
}

void x86_call(char* text, size_t* index, struct func* func) {
   char call[] = { 0xE8, 0x0, 0x0, 0x0, 0x0 };
   x86_write(text, index, call, sizeof(call));
   char cleanup[] = { 0x83, 0xC4 };
   x86_write(text, index, cleanup, sizeof(cleanup));
   char param = func->args*4;
   x86_write(text, index, (char*)&param, sizeof(param));
   char push[] = { 0x50 };
   x86_write(text, index, push, sizeof(push));
}


struct nr x86_compile(struct func* funcs) {
   struct nr nr;
   memset(&nr, 0, sizeof(nr));
   symbol_t* last = NULL;

   nr.text = malloc(4096);
   size_t text_index = 0;
   
   char frame_start[] = { 0x55, 0x89, 0xE5};
   char frame_stop[] = { 0x58, 0x5D, 0xC3};
   
   struct text_ref* refs = NULL;

   for (struct func* f = funcs; f != NULL; f = f->next) {
      if (f->first == NULL) continue;
      
      symbol_t* sym = malloc(sizeof(symbol_t));
      memset(sym, 0, sizeof(sym));
      sym->name = f->name;
      sym->text = text_index;
      sym->text_size = text_index;
   
      x86_write(nr.text, &text_index, frame_start, sizeof(frame_start));
     
      for (struct ins* ins = f->first; ins != NULL; ins = ins->next) {
         switch (ins->op) {
            case IR_LOADP:  {
               x86_loadp(nr.text, &text_index, f->args - ins->val - 1);
               break;
            }
            case IR_LOADV: {
               x86_loadv(nr.text, &text_index, ins->val);
               break;
             }
            case IR_CALL: {
               struct func* call = ins->func;
               if (strcmp("int_add", call->name) == 0) x86_add(nr.text, &text_index);
               else if (strcmp("int_sub", call->name) == 0) x86_sub(nr.text, &text_index);
               else if (strcmp("int_mul", call->name) == 0) x86_mul(nr.text, &text_index);
               else if (strcmp("int_div", call->name) == 0) x86_div(nr.text, &text_index);
               else { 
                  x86_call(nr.text, &text_index, call);
                  
                  struct text_ref* ref = malloc(sizeof(struct text_ref));
                  memset(ref, 0, sizeof(ref));
                  ref->callee = call->name;
                  ref->caller = text_index - 8;
                  
                  if (refs == NULL) {
                     refs = ref;
                  } else {
                     ref->next = refs;
                     refs = ref;
                  }
               }
               break;
            }
         }
      }
      
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
   
   return nr;
}
