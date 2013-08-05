#ifndef MAP_H
#define MAP_H

#include "list.h"
#include <string.h>

struct map_entry {
   void* key;
   size_t key_size;
   void* data;
   struct map_entry* next;
};

struct map {
 struct list l;
};

typedef struct map_entry map_it;


void map_init(struct map* m);
void map_clear(struct map* m);
int map_size(struct map* m);
void* map_get(struct map* m, void* key, size_t key_size);
void map_set(struct map* m, void* key, size_t key_size, void* data);
void map_foreach(struct map* m, void (*f)(void* key, size_t key_size, void* data));
map_it* map_iterator(struct map* m);
map_it* map_next(map_it* it);

#endif
