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

int list_size(struct list* l) {
   return l->size;
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
      if (l->first != NULL) {
         l->first->prev = NULL;
      }
   } 
   
   if (index == l->size - 1) {
      item = l->last;
      l->last = item->prev;
      if (l->last != NULL) {
         l->last->next = NULL;
      }
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

void* list_remove_item(struct list* l, void* data) {
   int i = 0;
   for (list_it* it = list_iterator(l); it != NULL; it = it->next) {
      if (it->data == data) {
         return list_remove(l, i);
      }
      i++;
   }

}

list_it* list_iterator(struct list* l) {
   return l->first;
}

list_it* list_next(list_it* it) {
   return it->next;
}

void* list_pop(struct list* l) {
   return list_remove(l,0);
}

bool list_eq(list_t* a, list_t* b) {
   if (list_size(a) != list_size(b)) {
      return false;
   }

   list_it* a_it = list_iterator(a);
   list_it* b_it = list_iterator(b);

   while (a_it != NULL) {
      if (a_it->data != b_it->data) {
         return false;
      }
   }
   return true;
}
