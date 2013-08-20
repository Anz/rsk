#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "list.h"
#include "ir.h"


//struct semantic_type semantic_check(struct ir_func* f, struct list* errors);
struct ir_func* semantic_check(struct map* funcs, struct ir_func* f, struct list args, struct list* errors);

#endif
