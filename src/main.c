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

extern struct ir_func* parse();
extern symbol_t* yysymbols();
extern char* yycode();

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

   // parse args
   while ((option = getopt(argc, argv,"i:o:")) != -1) {
      switch (option) {
          case 'i' : in = fopen(optarg, "r");
              break;
          case 'o' : out = fopen(optarg, "w");
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
   
   // compile into native code
   struct nr nr = x86_compile(funcs);
   
   elf_write(out, *nr.symbols, &nr.text, &nr.data);
   fclose(out);
   return 0;
}
