#ifndef OPT_H
#define OPT_H

#include "list.h"
#include "ir.h"


struct ir_func* optimize(struct map* funcs, struct ir_func* f, struct list args);

#endif
