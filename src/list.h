#ifndef LIST_H
#define LIST_H

#include <string.h>

struct list_item {
   void* data;
   struct list_item* prev;
   struct list_item* next;
};

struct list {
   struct list_item* first;
   struct list_item* last;
   size_t size;
};

void list_init(struct list* l);
void list_clear(struct list* l);
void* list_get(struct list* l, int index);
void list_add(struct list* l, void* data);
void* list_remove(struct list* l, int index);

#endif
