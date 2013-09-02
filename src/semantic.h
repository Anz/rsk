#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "list.h"
#include "ir.h"

struct semantic_type {
   struct ir_type* type;
   int param;
};

struct info {
   struct ir_func* func;
   struct semantic_type type;
};

void semantic_check(struct map* funcs, map_t* infos);

#endif
