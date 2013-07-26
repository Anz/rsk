%{

#include "map.h"
#include "elf.h"
#include "x86.h"
#include "ir.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PARSER_INIT(x) { size_t size = sizeof(*x); x = malloc(size); memset(x, 0, size); }

extern FILE * yyin;
extern int yylex();
extern int yylineno;

struct ir_func* func_first = NULL;

struct map* params_cur = NULL;

struct ir_type* type_int = NULL;
struct ir_type* type_float = NULL;
struct ir_type* type_array = NULL;

void yyerror(const char *str) {
   fprintf(stderr,"parser error on line %i: %s\n", yylineno,str);
   exit(1);
}

struct ir_func* func_get(char* name) {
   for (struct ir_func* f = func_first; f != NULL; f = f->next) {
      if (strcmp(f->name, name) == 0) {
         return f;
      }
   }
   return NULL;
}

struct ir_func* func_new(char* name) {
   struct ir_func* f = func_get(name);
   
   if (f == NULL) {
      f = malloc(sizeof(struct ir_func));
      memset(f, 0, sizeof(*f));
      f->name = strdup(name);
      f->next = func_first;
      func_first = f;
   }
   
   return f;
}

static void set_arg(struct ir_arg* arg, char* name) {
   int* index = (int*) map_get(params_cur, name, strlen(name) + 1);
   
   // parameter
   if (index != NULL) {
      arg->arg_type = IR_ARG_PARAM;
      arg->param = *index;
      
      arg->res = IR_RES_STA;
      arg->res_type = type_int;
      return;
   }
   
   // func
   struct ir_func* func = func_new(strdup(name));
   arg->arg_type = IR_ARG_CALL;
   arg->call.func = func;
   
   arg->res = IR_RES_STA;
   arg->res_type = type_array;
}

int yyparse ();

struct ir_func* parse(FILE* in) {
   // setup types
   type_int = malloc(sizeof(*type_int));
   type_int->add = func_new("int+");
   type_int->sub = func_new("int-");
   type_int->mul = func_new("int*");
   type_int->div = func_new("int/");
   
   type_float = malloc(sizeof(*type_float));
   type_float->add = func_new("float+");
   
   type_array = malloc(sizeof(*type_array));
   type_array->add = func_new("array+");

   // parsing
   yyin = in;
   yyparse();
   
   return func_first;
}

%}

%left  '+'  '-'
%left  '*'  '/'

%union {
   int word;
   char* str;
   struct {
      void* ptr;
      size_t size;
   } data;
   struct ir_func* func;
   struct map* params;
   struct ir_arg* arg;
   struct list* list;
}

%token ID INT FLOAT ARRAY

%type <func> line definition
%type <params> parameters parameter
%type <arg> expr term factor
%type <list> args
%type <str> ID '+' '-' '*' '/'
%type <word> INT FLOAT
%type <data> ARRAY

%%

line        : line definition
            | definition
            ;

definition  : ID parameters '=' expr         { $$ = func_new($1); $$->params = *$2; $$->value = *$4; }
            ;

parameters  : '(' parameter ')'              { $$ = $2; params_cur = $$; }
            |                                { PARSER_INIT($$); map_init($$); params_cur = $$; }
            ;

parameter   : parameter ',' ID               { int* index = malloc(4); *index = $1->l.size; map_set($1, strdup($3), strlen($3) + 1, index); }
            | ID                             { PARSER_INIT($$); map_init($$); int* index = malloc(4); *index = $$->l.size; map_set($$, strdup($1), strlen($1) + 1, index); }
            ;
            
expr        : expr '+' term                  { PARSER_INIT($$); $$->arg_type = IR_ARG_CALL; $$->res = IR_RES_STA; $$->res_type = $1->res_type; $$->call.func = func_new($2); list_add(&$$->call.args, $1); list_add(&$$->call.args, $3); }
            | expr '-' term                  { PARSER_INIT($$); $$->arg_type = IR_ARG_CALL; $$->res = IR_RES_STA; $$->res_type = $1->res_type; $$->call.func = func_new($2); list_add(&$$->call.args, $1); list_add(&$$->call.args, $3); }
            | term
            ;

term        : term '*' factor                { PARSER_INIT($$); $$->arg_type = IR_ARG_CALL; $$->res = IR_RES_STA; $$->res_type = $1->res_type; $$->call.func = func_new($2); list_add(&$$->call.args, $1); list_add(&$$->call.args, $3); }
            | term '/' factor                { PARSER_INIT($$); $$->arg_type = IR_ARG_CALL; $$->res = IR_RES_STA; $$->res_type = $1->res_type; $$->call.func = func_new($2); list_add(&$$->call.args, $1); list_add(&$$->call.args, $3);   }
            | factor
            ;

factor      : INT                            { PARSER_INIT($$); $$->arg_type = IR_ARG_WORD; $$->res = IR_RES_STA; $$->res_type = type_int; $$->word = $1; }
            | FLOAT                          { PARSER_INIT($$); $$->arg_type = IR_ARG_WORD; $$->res = IR_RES_STA; $$->res_type = type_float; $$->word = $1; }
            | ARRAY                          { PARSER_INIT($$); $$->arg_type = IR_ARG_DATA; $$->res = IR_RES_STA; $$->res_type = type_array; $$->data.ptr = $1.ptr; $$->data.size = $1.size; }
            | ID '(' args ')'                { PARSER_INIT($$); $$->arg_type = IR_ARG_CALL; $$->res = IR_RES_DYN; $$->res_type = NULL;$$->call.func = func_new(strdup($1)); $$->call.args = *$3; }
            | ID                             { PARSER_INIT($$); set_arg($$, $1); }
            ;

args        : args ',' expr                  { $$ = $1; list_add($$, $3); }
            | expr                           { PARSER_INIT($$); list_add($$, $1); }
            ;
%%
