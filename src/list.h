#ifndef LIST_H
#define LIST_H

#include <string.h>

struct list_item {
   void* data;
   struct list_item* prev;
   struct list_item* next;
};

typedef struct list {
   struct list_item* first;
   struct list_item* last;
   size_t size;
} list_t;

typedef struct list_item list_it;

void list_init(struct list* l);
void list_clear(struct list* l);
int list_size(struct list* l);
void* list_get(struct list* l, int index);
void list_add(struct list* l, void* data);
void* list_remove(struct list* l, int index);
void* list_remove_item(struct list* l, void* data);
list_it* list_iterator(struct list* l);
list_it* list_next(list_it* it);
void* list_pop(struct list* l);


#endif
