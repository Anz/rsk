#ifndef IR_H
#define IR_H

#define IR_LOADP  0x0
#define IR_LOADV  0x1
#define IR_CALL   0x2

struct func;

struct ins {
   int op;
   int val;
   struct func* func;
   struct ins* prev;
   struct ins* next;
};

struct func {
   char* name;
   int args;
   struct ins* first;
   struct ins* last;
   struct func* next;
};

// mnemonic
char* ir_dis(struct ins* i);

// generate code
void ir_loadp(struct func* f, int arg);
void ir_loadv(struct func* f, int val);
void ir_call(struct func* f, struct func* func);

#endif
