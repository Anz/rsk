#include "map.h"
#include <stdlib.h>

static struct map_entry* map_get_entry(struct map* m, void* key, size_t key_size) {
   for (int i = 0; i < m->l.size; i++) {
      struct map_entry* e = list_get(&m->l, i);
      
      if (e->key_size == key_size && memcmp(e->key, key, key_size) == 0) {
         return e;
      } 
   }
   
   return NULL;
}

void map_init(struct map* m) {
   list_init(&m->l);
}

void map_clear(struct map* m) {
   while (m->l.size > 0) {
      free(list_get(&m->l, 0));
      list_remove(&m->l, 0);
   }
}

void* map_get(struct map* m, void* key, size_t key_size) {
   struct map_entry* e = map_get_entry(m, key, key_size);
   
   if (e != NULL) {
      return e->data;
   }
   
   return NULL;
}

void map_set(struct map* m, void* key, size_t key_size, void* data) {
   struct map_entry* e = map_get_entry(m, key, key_size);
   
   if (e == NULL) {
      e = malloc(sizeof(struct map_entry));
      memset(e, 0, sizeof(*e));
      e->key = key;
      e->key_size = key_size;
      e->data = data;
   }
   
   list_add(&m->l, e);
}

