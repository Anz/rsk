#include "ir.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// mnemonic
char* ir_dis(struct ins* i) {
   static char name[10];

   switch(i->op) {
      case IR_LOADP: sprintf(name, "loadp %c", 97+i->val); break;
      case IR_LOADV: sprintf(name, "loadv %i", i->val); break;
      case IR_CALL: sprintf(name, "call %s", i->func->name); break;
      default: sprintf(name, "undef"); break;
   }
   
   return name;
}

void ir_push_ins(struct func* f, struct ins* ins) {
   if (f->first == NULL) {
      f->first = ins;
   }
   
   if (f->last != NULL) {
      f->last->next = ins;
   }
   
   ins->prev = f->last;
   f->last = ins;
}

void ir_loadp(struct func* f, int arg) {
   struct ins* ins = malloc(sizeof(struct ins));
   memset(ins, 0, sizeof(ins));
   ins->op = IR_LOADP;
   ins->val = arg;
   
   ir_push_ins(f, ins);
}

void ir_loadv(struct func* f, int val) {
   struct ins* ins = malloc(sizeof(struct ins));
   memset(ins, 0, sizeof(ins));
   ins->op = IR_LOADV;
   ins->val = val;
   
   ir_push_ins(f, ins);
}

void ir_call(struct func* f, struct func* func) {
   struct ins* ins = malloc(sizeof(struct ins));
   memset(ins, 0, sizeof(ins));
   ins->op = IR_CALL;
   ins->func = func;
   
   ir_push_ins(f, ins);
}
