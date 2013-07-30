#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "list.h"
#include "ir.h"

struct semantic_type {
   struct ir_type* type;
   int param;
};

struct semantic_type semantic_check(struct ir_func* f, struct list* errors);

#endif
