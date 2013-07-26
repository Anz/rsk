#include "elf.h"
#include "ir.h"
#include "x86.h"
#include "parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>

extern struct ir_func* parse();
extern symbol_t* yysymbols();
extern char* yycode();

void print_arg(struct ir_arg arg) {
   switch (arg.arg_type) {
      case IR_ARG_CALL:
         for (int i = 0; i < arg.call.args.size; i++) {
            struct ir_arg* a = (struct ir_arg*) list_get(&arg.call.args, i);
            print_arg(*a);
         }
         printf("\tcall %s\n", arg.call.func->name);
         break;
      case IR_ARG_WORD: printf("\tload #%i\n", arg.word); break;
      case IR_ARG_PARAM: printf("\tload %i\n", arg.param); break;
      case IR_ARG_DATA: printf("\tload '%s'\n", (char*)arg.data.ptr); break;
      default: printf("\tunknown %i\n", arg.arg_type); break;
   }
}

void print_func(struct ir_func* f) {
   if (f->value.arg_type == 0) {
      return;
   }

   printf("%s:\n", f->name);
   print_arg(f->value);
   printf("\n");
   
   /*struct ir_arg value = f->value;
   
   for (int i = 0; i >= 0; i--) {
      struct ir_arg arg =  *(struct ir_arg*) list_get(&args, i);
      x86_func_compile(refs, nr, arg, arg_count);
   }*/
}

void print_usage() {
    printf("Usage: rev [options] output files ...\n"
           "       -f, --format (elf|pe)\n"
           "       -a, --arch (x86| x64|arm)\n"
           "       -a, --api (linux|windows)\n"
           "       -o, --object (rel|lib|shared|exe)\n");
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
   
   // compile into intermediate code
   struct ir_func* funcs = parse(in);
   
   // print tree
   if (verbose) {
      for (struct ir_func* f = funcs; f != NULL; f = f->next) {
         print_func(f);
      }
   }
   
   // compile into native code
   struct nr nr = x86_compile(funcs);
   
   elf_write(out, *nr.symbols, &nr.text, &nr.data);
   fclose(out);
   return 0;
}
