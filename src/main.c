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

#include <signal.h>
#include <execinfo.h>


extern void parse(FILE*, struct map*);
extern char* yycode();

void bt_sighandler(int sig, siginfo_t *info,
                   void *secret);


void print_arg(struct ir_arg* arg) {
   switch (arg->arg_type) {
      case IR_ARG_CALL:
         for (int i = 0; i < arg->call.args.size; i++) {
            struct ir_arg* a = (struct ir_arg*) list_get(&arg->call.args, i);
            print_arg(a);
         }
         
         struct ir_func* f = arg->call.func;
         printf("\t(type: %s)\tcall %s\n", f->type->name, f->name);
         
         break;
      case IR_ARG_PARAM:
         printf("\t(type: unknown)\tload %s\n", arg->call.param->name);
         break;
      case IR_ARG_DATA:
         if (arg->data.size == 4) {
            printf("\t(type: %s)\tload #%i\n", arg->data.type->name, arg->data.word); 
         } else {
            printf("\t(type: %s)\tload '%s'\n", arg->data.type->name, (char*)arg->data.ptr);
         }
         break;
      default: printf("\tunknown %i\n", arg->arg_type); break;
   }
}

void print_func(struct ir_func* f) {
   if (list_size(&f->cases) == 0) {
      return;
   }
   
   printf("%s %s", f->type->name, f->name);
   
   if (f->params.l.size > 0) {
      map_it* it = map_iterator(&f->params);
      struct ir_param* param = (struct ir_param*) it->data;
      
      printf("(%s %s", "unknown", param->name);
      
      it = map_next(it);
      while (it != NULL) {
         param = (struct ir_param*) it->data;
         printf(", %s %s", "unknown", param->name);
         it = map_next(it);
      }
      printf(")");
   }
   printf(":\n");
   
   print_arg(((struct ir_case*)list_get(&f->cases, 0))->func);
   printf("\n");
}

void print_usage() {
    printf("Usage: rev [options] output files ...\n"
           "       -i <input>    source file\n"
           "       -o <ouput>    output binary\n"
           "       -v            verbose\n");
}

int main (int argc, char *argv[]) {
   struct sigaction sa;

  sa.sa_sigaction = (void *)bt_sighandler;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = SA_RESTART | SA_SIGINFO;

  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGUSR1, &sa, NULL);

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
   
   struct list args;
   list_init(&args);
   
   semantic_check(&funcs, map_get(&funcs, "main", 5), args, &errors);
   for (map_it* it = map_iterator(&funcs); it != NULL; it = it->next) {
      struct ir_func* f = it->data;
      if (f->type == NULL) {
         map_set(&funcs, f->name, strlen(f->name)+1, NULL);
      }
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
   /*for (map_it* it = map_iterator(&funcs); it != NULL; it = map_next(it)) {
      struct ir_func* f = it->data;
      ir_func_clear(f);
   }
   map_clear(&funcs);*/
   
   // free native representation
   //x86_free(nr);
   //nr.symbols = NULL;
   
   return 0;
}

/* get REG_EIP from ucontext.h */
#include <ucontext.h>

void bt_sighandler(int sig, siginfo_t *info,
                   void *secret) {

  void *trace[16];
  char **messages = (char **)NULL;
  int i, trace_size = 0;
  ucontext_t *uc = (ucontext_t *)secret;

  /* Do something useful with siginfo_t */
  if (sig == SIGSEGV)
    printf("Got signal %d, faulty address is %p, "
           "from %p\n", sig, info->si_addr, 
           (void*)uc->uc_mcontext.gregs[REG_EIP]);
  else
    printf("Got signal %d#92;\n", sig);

  trace_size = backtrace(trace, 16);
  /* overwrite sigaction with caller's address */
  trace[1] = (void *) uc->uc_mcontext.gregs[REG_EIP];

  messages = backtrace_symbols(trace, trace_size);
  /* skip first stack frame (points here) */
  for (i=1; i<trace_size; ++i)
  {
    char syscom[256];
    sprintf(syscom,"addr2line %p -e %s", trace[i] , "bin/rev" ); //last parameter is the name of this app
    system(syscom);
  }
  exit(0);
}
