#ifndef BUFFER_H
#define BUFFER_H

#include <string.h>

typedef struct buffer {
   char* data;
   size_t capacity;
   size_t size;
} buffer_t;

void buffer_init(struct buffer* buf, size_t capacity);
void buffer_free(struct buffer* buf);
void buffer_copy(struct buffer* src, struct buffer* dest);
void buffer_write(struct buffer* buf, void* data, size_t size);
void buffer_writes(struct buffer* buf, char* str, ...);
void buffer_writew(struct buffer* buf, int word);
void buffer_writeb(struct buffer* buf, char byte);

#endif
