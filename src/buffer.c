#include "buffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

static void inc(struct buffer* buf) {
   buf->capacity *= 2;
   buf->data = realloc(buf->data, buf->capacity); 
};

static void check_bounds(struct buffer* buf, size_t size) {
   if (buf->size + size > buf->capacity) {
      inc(buf);
   }
}

void buffer_init(struct buffer* buf, size_t capacity) {
   buf->capacity = capacity;
   buf->data = malloc(buf->capacity);
   buf->size = 0;
}

void buffer_free(struct buffer* buf) {
   free(buf->data);
   memset(buf, 0, sizeof(buf));
}

void buffer_copy(struct buffer* src, struct buffer* dest) {
   buffer_write(dest, src->data, src->size);
}

void buffer_write(struct buffer* buf, void* data, size_t size) {
   check_bounds(buf, size);
   memcpy(buf->data + buf->size, data, size);
   buf->size += size;
}

void buffer_writes(struct buffer* buf, char* str, ...) {
   check_bounds(buf, strlen(str)+512); // for arguments
   
   va_list args;
   va_start(args, str);
   buf->size += vsprintf(buf->data + buf->size, str, args);
   va_end(args);
}

void buffer_writew(struct buffer* buf, int word) {
   buffer_write(buf, &word, sizeof(word));
}

void buffer_writeb(struct buffer* buf, char byte) {
   buffer_write(buf, &byte, sizeof(byte));
}
