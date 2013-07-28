#ifndef MAP_H
#define MAP_H

#include "list.h"
#include <string.h>

struct map_entry {
   void* key;
   size_t key_size;
   void* data;
};

struct map {
 struct list l;
};

void map_init(struct map* m);
void map_clear(struct map* m);
void* map_get(struct map* m, void* key, size_t key_size);
void map_set(struct map* m, void* key, size_t key_size, void* data);
void map_foreach(struct map* m, void (*f)(void* key, size_t key_size, void* data));

#endif
