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

struct map* funcs = NULL;

struct map* params_cur = NULL;

struct ir_type* type_int = NULL;
struct ir_type* type_float = NULL;
struct ir_type* type_array = NULL;

void yyerror(const char *str) {
   fprintf(stderr,"parser error on line %i: %s\n", yylineno,str);
   exit(1);
}

struct ir_func* func_new(char* name) {
   struct ir_func* f = map_get(funcs, name, strlen(name) + 1);
   
   if (f == NULL) {
      f = malloc(sizeof(*f));
      memset(f, 0, sizeof(*f));
      f->name = strdup(name);
      
      map_set(funcs, f->name, strlen(f->name) + 1, f);
   }
   
   return f;
}

static void set_arg(struct ir_arg* arg, char* name) {
   int* index = (int*) map_get(params_cur, name, strlen(name) + 1);
   
   // parameter
   if (index != NULL) {
      arg->arg_type = IR_ARG_PARAM;
      arg->param = *index;
      
      arg->type.type = IR_RES_DYN;
      arg->type.dyn = *index;
      return;
   }
   
   // func
   struct ir_func* func = func_new(name);
   arg->arg_type = IR_ARG_CALL;
   arg->call.func = func;
   
   arg->type.type = IR_RES_UNK;
}

int yyparse ();

void parse(FILE* in, struct map* f) {
   // init structures
   //map_init(&funcs);
   funcs = f;

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

definition  : ID parameters '=' expr         { $$ = func_new($1); $$->params = *$2; $$->value = $4; $$ = map_get(funcs, $1, strlen($1)+1); }
            ;

parameters  : '(' parameter ')'              { $$ = $2; params_cur = $$; }
            |                                { PARSER_INIT($$); map_init($$); params_cur = $$; }
            ;

parameter   : parameter ',' ID               { int* index = malloc(4); *index = $1->l.size; map_set($1, strdup($3), strlen($3) + 1, index); }
            | ID                             { PARSER_INIT($$); map_init($$); int* index = malloc(4); *index = $$->l.size; map_set($$, strdup($1), strlen($1) + 1, index); }
            ;
            
expr        : expr '+' term                  { PARSER_INIT($$); $$->arg_type = IR_ARG_CALL; $$->call.func = func_new($2); list_add(&$$->call.args, $1); list_add(&$$->call.args, $3); }
            | expr '-' term                  { PARSER_INIT($$); $$->arg_type = IR_ARG_CALL; $$->call.func = func_new($2); list_add(&$$->call.args, $1); list_add(&$$->call.args, $3); }
            | term
            ;

term        : term '*' factor                { PARSER_INIT($$); $$->arg_type = IR_ARG_CALL; $$->call.func = func_new($2); list_add(&$$->call.args, $1); list_add(&$$->call.args, $3); }
            | term '/' factor                { PARSER_INIT($$); $$->arg_type = IR_ARG_CALL; $$->call.func = func_new($2); list_add(&$$->call.args, $1); list_add(&$$->call.args, $3);   }
            | factor
            ;

factor      : INT                            { PARSER_INIT($$); $$->arg_type = IR_ARG_WORD; $$->type.type = IR_RES_STA; $$->type.sta = type_int; $$->word = $1; }
            | FLOAT                          { PARSER_INIT($$); $$->arg_type = IR_ARG_WORD; $$->type.type = IR_RES_STA; $$->type.sta = type_float; $$->word = $1; }
            | ARRAY                          { PARSER_INIT($$); $$->arg_type = IR_ARG_DATA; $$->type.type = IR_RES_STA; $$->type.sta = type_array; $$->data.ptr = $1.ptr; $$->data.size = $1.size; }
            | ID '(' args ')'                { PARSER_INIT($$); $$->arg_type = IR_ARG_CALL; $$->call.func = func_new($1); $$->call.args = *$3; }
            | ID                             { PARSER_INIT($$); set_arg($$, $1); }
            ;

args        : args ',' expr                  { $$ = $1; list_add($$, $3); }
            | expr                           { PARSER_INIT($$); list_add($$, $1); }
            ;
%%
