#include "map.h"
//#include "elf.h"
#include "ir.h"
//#include "x86.h"
#include "i32.h"
#include "parser.h"
#include "semantic.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>

extern void parse(FILE*, struct map*);
extern char* yycode();

static char param_type(int category) {
   return (char) (65 + category);
}

static char* arg_type(struct ir_type* type, struct ir_param* param) {
   if (type != NULL) {
      return type->name;
   } else {
      char* str = malloc(2);
      memset(str, 0, 2);
      str[0] = param_type(param->category);
      return str;
   }
   return "";
}

void print_arg(struct ir_arg* arg) {
   switch (arg->arg_type) {
      case IR_ARG_CALL:
         for (int i = 0; i < arg->call.args.size; i++) {
            struct ir_arg* a = (struct ir_arg*) list_get(&arg->call.args, i);
            print_arg(a);
         }
         
         struct ir_func* f = arg->call.func;
         printf("\t(type: %s)\tcall %s\n", arg_type(ir_arg_type(arg), ir_arg_param(arg)), f->name);
         
         break;
      case IR_ARG_PARAM:
         printf("\t(type: %s)\tload %s\n", arg_type(arg->call.param->type, arg->call.param), arg->call.param->name);
         break;
      case IR_ARG_DATA:
         if (arg->data.size == 4) {
            printf("\t(type: %s)\tload #%i\n", arg_type(arg->data.type, NULL), arg->data.word); 
         } else {
            printf("\t(type: %s)\tload '%s'\n", arg_type(arg->data.type, NULL), (char*)arg->data.ptr);
         }
         break;
      default: printf("\tunknown %i\n", arg->arg_type); break;
   }
}

void print_func(struct ir_func* f) {
   if (f->value == NULL) {
      return;
   }
   
   struct ir_param* param = NULL;
   if (f->type_param < f->params.l.size) {
      param = ((struct map_entry*)list_get(&f->params.l, f->type_param))->data;
   }
   
   printf("%s %s", arg_type(f->type, param), f->name);
   
   if (f->params.l.size > 0) {
      map_it* it = map_iterator(&f->params);
      struct ir_param* param = (struct ir_param*) it->data;
      
      printf("(%s %s", arg_type(param->type, param), param->name);
      
      it = map_next(it);
      while (it != NULL) {
         param = (struct ir_param*) it->data;
         printf(", %s %s", arg_type(param->type, param), param->name);
         it = map_next(it);
      }
      printf(")");
   }
   printf(":\n");
   
   print_arg(f->value);
   printf("\n");
}

void print_usage() {
    printf("Usage: rev [options] output files ...\n"
           "       -i <input>    source file\n"
           "       -o <ouput>    output binary\n"
           "       -v            verbose\n");
}

int main (int argc, char *argv[]) {
   int option = 0;
   FILE* in = NULL;
   FILE* out = stdout;
   bool verbose = false;

   // parse args
   while ((option = getopt(argc, argv,"i:o:v")) != -1) {
      switch (option) {
          case 'i' : in = fopen(optarg, "r");
              break;
          case 'o' : out = fopen(optarg, "w");
              break;
          case 'v' : verbose = true;
              break;
          default: print_usage(); 
              exit(EXIT_FAILURE);
      }
   }
   
   // print usage
   if (in == NULL) {
      print_usage();
      return 0;
   }
   
   // error list
   struct list errors;
   list_init(&errors);
   
   // function map
   struct map funcs;
   map_init(&funcs);
   
   // compile into intermediate representation
   parse(in, &funcs);
   fclose(in);
   
   // do semantic check
   for (map_it* it = map_iterator(&funcs); it != NULL; it = map_next(it)) {
      semantic_check((struct ir_func*)it->data, &errors);
   }
   
   // print tree
   if (verbose) {
      for (struct list_item* item = funcs.l.first; item != NULL; item = item->next) {
         struct map_entry* entry = (struct map_entry*) item->data;
         print_func(entry->data);
      }
   }
   
   // print error
   if (errors.size > 0) {
      for (list_it* it = list_iterator(&errors); it != NULL; it = list_next(it)) {
         struct ir_error* error = (struct ir_error*)it->data;
         ir_print_err(*error);
         free(error);
      }
      
      // cleanup
      list_clear(&errors);
      fclose(out);
      
      return 1;
   }
   list_clear(&errors);

   // compile into native code
   struct buffer buf = i32_compile(funcs);
   fwrite(buf.data, buf.size, 1, out);
   buffer_free(&buf);
   /*struct nr nr = x86_compile(funcs);

   elf_write(out, *nr.symbols, &nr.text, &nr.data);
   fclose(out);*/
   
   // free functions
   for (map_it* it = map_iterator(&funcs); it != NULL; it = map_next(it)) {
      struct ir_func* f = it->data;
      ir_func_clear(f);
   }
   map_clear(&funcs);
   
   // free native representation
   //x86_free(nr);
   //nr.symbols = NULL;
   
   return 0;
}
