#include "list.h"
#include <stdlib.h>

static struct list_item* list_get_element(struct list* l, int index) {
   if (index < 0 || index >= l->size) {
      return NULL;
   }
   
   struct list_item* item = l->first;
   for (int i = 0; i < index; i++) {
      item = item->next;
   }
   return item;
}

void list_init(struct list* l) {
   memset(l, 0, sizeof(*l));
}

void list_clear(struct list* l) {
   struct list_item* prev = NULL;
   for (struct list_item* i = l->first; i != NULL; i = i->next) {
      if (prev != NULL) {
         free(prev);
      }
      prev = i;
   }
   
   memset(l, 0, sizeof(*l));
}

void* list_get(struct list* l, int index) {
   struct list_item* item = list_get_element(l, index);
   
   if (item == NULL) {
      return NULL;
   }
   
   return item->data;
}

void list_add(struct list* l, void* data) {
   struct list_item* item = malloc(sizeof(struct list_item));
   memset(item, 0, sizeof(*item));
   item->data = data;
   
   struct list_item* last = l->last;
   
   if (last == NULL) {
      l->first = item;
      l->last = item;
   } else {
      item->prev = last;
      last->next = item;
      l->last = item;
   }
   
   l->size++;
}

void* list_remove(struct list* l, int index) {
   if (index < 0 || index >= l->size) {
      return NULL;
   }
   
   struct list_item* item = NULL;
   

   if (index == 0) {
      item = l->first;
      l->first = item->next;
      l->first->prev = NULL;
   } 
   
   if (index == l->size - 1) {
      item = l->last;
      l->last = item->prev;
      l->last->next = NULL;
   }
   
   if (item == NULL) {
      item = list_get_element(l, index);
      struct list_item* prev = item->prev;
      struct list_item* next = item->next;
      prev->next = next;
      next->prev = prev;
   }
   
   // get data
   void* data = item->data;
   
   // free item
   free(item);
   
   // reduce size
   l->size--;
   
   return data;
}

