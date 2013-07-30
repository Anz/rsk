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
   for (int i = 0; i < m->l.size; i++) {
      free(list_get(&m->l, i));
   }
   
   list_clear(&m->l);
}

void* map_get(struct map* m, void* key, size_t key_size) {
   struct map_entry* e = map_get_entry(m, key, key_size);
   void* data = NULL;
   
   if (e != NULL) {
      data = e->data;
   }
   
   return data;
}

void map_set(struct map* m, void* key, size_t key_size, void* data) {
   struct map_entry* e = map_get_entry(m, key, key_size);
   
   if (e == NULL) {
      e = malloc(sizeof(struct map_entry));
      memset(e, 0, sizeof(*e));
      e->key = key;
      e->key_size = key_size;
      
      if (m->l.last != NULL) {
         ((struct map_entry*) m->l.last->data)->next = e;
      }
      list_add(&m->l, e);
   }
   
   e->data = data;
}

void map_foreach(struct map* m, void (*f)(void* key, size_t key_size, void* data)) {
   for (struct list_item* item = m->l.first; item != NULL; item = item->next) {
      struct map_entry* entry = (struct map_entry*) item->data;
      f(entry->key, entry->key_size, entry->data);
   }
}

map_it* map_iterator(struct map* m) {
   if (m->l.size == 0) {
      return NULL;
   }
   
   return m->l.first->data;
}

map_it* map_next(map_it* it) {
   return it->next;
}
