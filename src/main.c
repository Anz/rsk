#include "map.h"
#include "elf.h"
#include "ir.h"
#include "x86.h"
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
extern symbol_t* yysymbols();
extern char* yycode();

void print_arg(struct ir_arg* arg) {
   switch (arg->arg_type) {
      case IR_ARG_CALL:
         for (int i = 0; i < arg->call.args.size; i++) {
            struct ir_arg* a = (struct ir_arg*) list_get(&arg->call.args, i);
            print_arg(a);
         }
         printf("\tcall %s", arg->call.func->name);
         break;
      case IR_ARG_WORD: printf("\tload #%i", arg->word); break;
      case IR_ARG_PARAM: printf("\tload %i", arg->param); break;
      case IR_ARG_DATA: printf("\tload '%s'", (char*)arg->data.ptr); break;
      default: printf("\tunknown %i", arg->arg_type); break;
   }
   
   switch (arg->res) {
      case IR_RES_STA: printf(" (type: %s)\n", arg->res_type->add->name); break;
      case IR_RES_DYN: printf(" (type: param #%i)\n", arg->res_param); break;
      default: printf(" (type: unknown\n");
   }
}

void print_func(struct ir_func* f) {
   if (f->value == NULL) {
      return;
   }

   printf("%s:\n", f->name);
   print_arg(f->value);
   printf("\n");
}

static void do_semantic_check(void* key, size_t key_size, void* data) {
   semantic_check((struct ir_func*)data);
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
   if (in == NULL) {
      print_usage();
      return 0;
   }
   
   // function map
   struct map funcs;
   map_init(&funcs);
   
   // compile into intermediate representation
   parse(in, &funcs);
   fclose(in);
   
   // do semantic check
   map_foreach(&funcs, do_semantic_check);
   
   // print tree
   if (verbose) {
      for (struct list_item* item = funcs.l.first; item != NULL; item = item->next) {
         struct map_entry* entry = (struct map_entry*) item->data;
         print_func(entry->data);
      }
   }
   
   // compile into native code
   struct nr nr = x86_compile(funcs);

   elf_write(out, *nr.symbols, &nr.text, &nr.data);
   fclose(out);
   
   // TODO: free all func names!
   map_clear(&funcs);
   
   return 0;
}
