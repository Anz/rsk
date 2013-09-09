#ifndef OPT_H
#define OPT_H

#include "list.h"
#include "ir.h"
#include "semantic.h"


struct ir_type* optimize(struct map* funcs, map_t* info, struct ir_func* f, struct list args);

#endif
