#include "i32.h"
#include "asm.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

// include x86 native routines
#include "x86_fun.c"

int data_id = 0;
int argidx = 0;

void x86_func_compile(buffer_t* text, buffer_t* data, struct ir_arg* arg, int arg_count);

void x86_loadp(struct buffer* text, int param) {
   /*buffer_writes(buf, 
      "\tpush %i(%%ebp)\n",
      (1+param)*4);*/
      
   int index = (1+param)*4;
      
   switch (argidx) {
      case 0: buffer_writes(text, "\tmovl %i(%%ebp), %%eax\n", index); break;
      case 1: buffer_writes(text, "\tmovl %i(%%ebp), %%ebx\n", index); break;
      case 2: buffer_writes(text, "\tmovl %i(%%ebp), %%ecx\n", index); break;
      case 3: buffer_writes(text, "\tmovl %i(%%ebp), %%edx\n", index); break;
      default: buffer_writes(text, "\tpush %i(%%ebp)\n", index); break;
   }
   
   argidx++;
}

void x86_loadv(buffer_t* text, int arg) {
   //buffer_writes(text, "\tpush $%i\n", arg);
   
   switch (argidx) {
      case 0: buffer_writes(text, "\tmovl $%i, %%eax\n", arg); break;
      case 1: buffer_writes(text, "\tmovl $%i, %%ebx\n", arg); break;
      case 2: buffer_writes(text, "\tmovl $%i, %%ecx\n", arg); break;
      case 3: buffer_writes(text, "\tmovl $%i, %%edx\n", arg); break;
      default: buffer_writes(text, "\tpush $%i\n", arg); break;
   }
   
   argidx++;
}

void x86_loadd(buffer_t* text, char* label) {
   /*buffer_writes(text, 
      "\tpush $%s\n",
      label);*/
         
  /*buffer_writes(text,
         "\tcall _print\n"
         "\tadd $4, %%esp\n");*/
         
  switch (argidx) {
      case 0: buffer_writes(text, "\tmovl $%s, %%eax\n", label); break;
      case 1: buffer_writes(text, "\tmovl $%s, %%ebx\n", label); break;
      case 2: buffer_writes(text, "\tmovl $%s, %%ecx\n", label); break;
      case 3: buffer_writes(text, "\tmovl $%s, %%edx\n", label); break;
      default: buffer_writes(text, "\tpush $%s\n", label); break;
   }
   
   argidx++;
}

void x86_int_add(buffer_t* text, buffer_t* data, struct ir_arg* a, struct ir_arg* b, int arg_count) {
   x86_func_compile(text, data, a, arg_count);
   x86_func_compile(text, data, b, arg_count);
   
   buffer_writes(text, 
      /*"\tpop %%eax\n"
      "\tpop %%edx\n"*/
      "\tadd %%edx, %%eax\n"
      /*"\tpush %%eax\n"*/);
}

void x86_int_sub(buffer_t* text, buffer_t* data, struct ir_arg* a, struct ir_arg* b, int arg_count) {
   x86_func_compile(text, data, a, arg_count);
   x86_func_compile(text, data, b, arg_count);
   
   buffer_writes(text, 
      /*"\tpop %%eax\n"
      "\tpop %%edx\n"*/
      "\tsub %%edx, %%eax\n"
      /*"\tpush %%eax\n"*/);
}

void x86_int_mul(buffer_t* text, buffer_t* data, struct ir_arg* a, struct ir_arg* b, int arg_count) {
   x86_func_compile(text, data, a, arg_count);
   x86_func_compile(text, data, b, arg_count);
   
   buffer_writes(text, 
      /*"\tpop %%eax\n"
      "\tpop %%edx\n"*/
      "\tmul %%edx, %%eax\n"
      /*"\tpush %%eax\n"*/);
}

void x86_int_div(buffer_t* text, buffer_t* data, struct ir_arg* a, struct ir_arg* b, int arg_count) {
   x86_func_compile(text, data, a, arg_count);
   x86_func_compile(text, data, b, arg_count);
   
   buffer_writes(text, 
      /*"\tpop %%eax\n"
      "\tpop %%edx\n"*/
      "\tdiv %%edx, %%eax\n"
      /*"\tpush %%eax\n"*/);
}

void x86_int_cmp(buffer_t* text, buffer_t* data, struct ir_arg* a, struct ir_arg* b, int arg_count) {
   x86_func_compile(text, data, a, arg_count);
   x86_func_compile(text, data, b, arg_count);
   
   buffer_writes(text, 
      /*"\tpop %%eax\n"
      "\tpop %%edx\n"*/
      "\tadd %%edx, %%eax\n"
      /*"\tpush %%eax\n"*/);
}

void x86_array_add(buffer_t* text, buffer_t* data, struct ir_arg* a, struct ir_arg* b, int arg_count) {
   x86_func_compile(text, data, a, arg_count);   
   x86_func_compile(text, data, b, arg_count);
   
   /*buffer_writes(text,
         "\tpush %%eax\n"
         "\tcall _print\n"
         "\tpop %%eax\n");*/
   
   buffer_writes(text,
      "\tpush %%ebx\n"
      "\tpush %%eax\n"
      "\tcall _concat\n"
      "\tadd $8, %%esp\n");
}

void* funcs_ow[][2] = {
   { "int+", &x86_int_add },
   { "int-", &x86_int_sub },
   { "int*", &x86_int_mul },
   { "int/", &x86_int_div },
   { "int=", &x86_int_cmp },
   { "int<", &x86_int_cmp },
   { "int>", &x86_int_cmp },
   { "float+", &x86_int_add },
   { "array+", &x86_array_add },
};

void x86_save_reg(buffer_t* text, int args) {
   switch (args) {
      case 0: break;
      case 1:  buffer_writes(text, "\tpush %%eax\n"); break;
      case 2:  buffer_writes(text, "\tpush %%eax\n\tpush %%ebx\n"); break;
      case 3:  buffer_writes(text, "\tpush %%eax\n\tpush %%ebx\n\tpush %%ecx\n"); break;
      default:  buffer_writes(text, "\tpush %%eax\n\tpush %%ebx\n\tpush %%ecx\n\tpush %%edx\n"); break;
   }
}

void x86_restore_reg(buffer_t* text, int args) {
   switch (args) {
      case 0: break;
      case 1:  buffer_writes(text, "\tmov %%eax, %%ebx\n\tpop %%eax\n"); break;
      case 2:  buffer_writes(text, "\tmov %%eax, %%ecx\n\tpop %%ebx\n\tpop %%eax\n"); break;
      case 3:  buffer_writes(text, "\tmov %%eax, %%edx\n\tpop %%ecx\n\tpop %%ebx\n\tpop %%eax\n"); break;
      default:  buffer_writes(text, "\tmov %%eax, %%esi\n\tpop %%edx\n\tpop %%ecx\n\tpop %%ebx\n\tpush %%eax\n\tpush %%esi\n"); break;
   }
}

void x86_call(buffer_t* text, buffer_t* data, struct ir_arg* arg, int arg_count) {
   int tmpargidx = argidx;
   argidx = 0;
   
   x86_save_reg(text, tmpargidx);
 
   struct ir_func* func = arg->call.func;
   struct ir_arg* left_op = list_get(&arg->call.args, 0);

   // native function overwrite
   for (int i = 0; i < sizeof(funcs_ow) / sizeof(funcs_ow[0]); i++) {
      char* name = (char*)funcs_ow[i][0];
      void (*f)(buffer_t*, buffer_t*, struct ir_arg*, struct ir_arg*, int) = funcs_ow[i][1];
      if (strcmp(name, func->name) == 0) {
         struct ir_arg* a = list_get(&arg->call.args, 0);
         struct ir_arg* b = list_get(&arg->call.args, 1);
         f(text, data, a, b, arg_count);
         x86_restore_reg(text, tmpargidx);
         argidx = tmpargidx + 1;
         return;
      }
   }
   
   for (int i = arg->call.args.size - 1; i >= 0; i--) {
      struct ir_arg* a =  (struct ir_arg*) list_get(&arg->call.args, i);
      x86_func_compile(text, data, a, arg_count);
   }
   
   // generic function
   char call[512];
   memset(call, 0, sizeof(call));
   sprintf(call, 
      "\tcall %s\n"
      "\tadd $%i, %%esp\n",
      func->name, 4*list_size(&func->params.l));
   buffer_write(text, call, strlen(call));
   
   x86_restore_reg(text, tmpargidx);
   argidx = tmpargidx + 1;
}

static void escape(char* dest, char* src) {
   int i = 0;
   for (; src[i] != '\0'; i++) {
      if (src[i] == '\n') {
         dest[i++] = '\\';
         dest[i] = 'n';
      } else {
         dest[i] = src[i];
      }
   }
   dest[i] = src[i];
}

void x86_func_compile(buffer_t* text, buffer_t* data, struct ir_arg* arg, int arg_count) {
   switch (arg->arg_type) {
      case IR_ARG_CALL: {         
         x86_call(text, data, arg, arg_count);
         break;
      }
      case IR_ARG_PARAM: {
         x86_loadp(text, arg_count - arg->call.param->index);
         break;
      }
      case IR_ARG_DATA: {
         if (arg->data.size == 4) {
            x86_loadv(text, arg->data.word);
         } else {
            char name[512];
            
            union {
               int len;
               struct {
                  char a, b, c, d;
               };
            } len;
            sprintf(name, "_data%04i", data_id++);
            len.len = arg->data.size;
            x86_loadd(text, name);
            
            buffer_writes(data, "%s:\t.byte 0x%02x, 0x%02x, 0x%02x, 0x%02x", name, len.a, len.b, len.c, len.d);
            for (int i = 0; i < len.len; i++) {
               buffer_writes(data, ", 0x%02x", (int)((char*)arg->data.ptr)[i]);
            }
            buffer_writes(data, "\n");
         }
         break;
      }
   }
}

struct buffer i32_compile(struct map funcs) {

   struct buffer text;
   buffer_init(&text, 4096);
   
   struct buffer data;
   buffer_init(&data, 512);
   buffer_writes(&data, ".data\n");
   
   // write native functions to buffer 
   buffer_write(&text, x86_fun_s, sizeof(x86_fun_s));
   
   for (struct list_item* item = funcs.l.first; item != NULL; item = item->next) {
      struct map_entry* entry = (struct map_entry*) item->data;
      struct ir_func* f = entry->data;
   
      if (f->value == NULL) {
         continue;
      }
      
      argidx = 0;
   
      buffer_writes(&text,
         "\n%s:\n"
         "\tpush %%ebp\n"
         "\tmov %%esp, %%ebp\n",
         f->name);
      
      x86_func_compile(&text, &data, f->value, map_size(&f->params));
      
      buffer_writes(&text,
         "\tmov %%ebp, %%esp\n"
         "\tpop %%ebp\n"
         "\tret\n");
   }
   
   buffer_copy(&data, &text);
   
   return text;
}
