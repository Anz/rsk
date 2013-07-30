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
   struct ir_param* param = (struct ir_param*) map_get(params_cur, name, strlen(name) + 1);
   
   // parameter
   if (param != NULL) {
      arg->arg_type = IR_ARG_PARAM;
      arg->param = param;
      return;
   }
   
   // func
   struct ir_func* func = func_new(name);
   arg->arg_type = IR_ARG_CALL;
   arg->call.func = func;
}

int yyparse ();

static struct ir_func* binary_op(char* name, struct ir_type* type) {
   struct ir_param* a = malloc(sizeof(*a));
   memset(a, 0, sizeof(*a));
   a->name = "a";
   a->type = type;
   
   struct ir_param* b = malloc(sizeof(*b));
   memset(b, 0, sizeof(*b));
   b->name = "b";
   b->type = type;
   
   
   // setup functions
   struct ir_func* f = func_new(name);
   f->type = type;
   map_set(&f->params, a->name, strlen(a->name)+1, a);
   map_set(&f->params, b->name, strlen(b->name)+1, b);
   
   return f;
}

void parse(FILE* in, struct map* f) {
   // init structures
   funcs = f;
   
   // setup functions
   binary_op("+", NULL);
   

   // setup types
   type_int = malloc(sizeof(*type_int));
   type_int->name = "int";
   type_int->add = binary_op("int+", type_int);   
   type_int->sub = binary_op("int-", type_int);
   type_int->mul = binary_op("int*", type_int);
   type_int->div = binary_op("int/", type_int);
   
   type_float = malloc(sizeof(*type_float));
   type_float->name = "float";
   type_float->add = binary_op("float+", type_float);
   
   type_array = malloc(sizeof(*type_array));
   type_array->name = "array";
   type_array->add = binary_op("array+", type_array);

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

definition  : ID parameters '=' expr         { $$ = func_new($1); $$->lineno = yylineno; $$->params = *$2; $$->value = $4; $$ = map_get(funcs, $1, strlen($1)+1); }
            ;

parameters  : '(' parameter ')'              { $$ = $2; params_cur = $$; }
            |                                { PARSER_INIT($$); map_init($$); params_cur = $$; }
            ;

parameter   : parameter ',' ID               { $$ = $1; struct ir_param* param; PARSER_INIT(param); param->lineno = yylineno; param->name = strdup($3); param->index = $1->l.size; param->category = param->index; map_set($1, param->name, strlen(param->name) + 1, param); }
            | ID                             { PARSER_INIT($$); map_init($$); struct ir_param* param; PARSER_INIT(param); param->lineno = yylineno; param->name = strdup($1); param->index = $$->l.size; param->category = param->index; map_set($$, param->name, strlen(param->name) + 1, param); }
            ;
            
expr        : expr '+' term                  { PARSER_INIT($$); $$->lineno = yylineno; $$->arg_type = IR_ARG_CALL; $$->call.func = func_new($2); list_add(&$$->call.args, $1); list_add(&$$->call.args, $3); }
            | expr '-' term                  { PARSER_INIT($$); $$->lineno = yylineno; $$->arg_type = IR_ARG_CALL; $$->call.func = func_new($2); list_add(&$$->call.args, $1); list_add(&$$->call.args, $3); }
            | term
            ;

term        : term '*' factor                { PARSER_INIT($$);$$->lineno = yylineno;  $$->arg_type = IR_ARG_CALL; $$->call.func = func_new($2); list_add(&$$->call.args, $1); list_add(&$$->call.args, $3); }
            | term '/' factor                { PARSER_INIT($$);$$->lineno = yylineno;  $$->arg_type = IR_ARG_CALL; $$->call.func = func_new($2); list_add(&$$->call.args, $1); list_add(&$$->call.args, $3);   }
            | factor
            ;

factor      : INT                            { PARSER_INIT($$); $$->lineno = yylineno; $$->arg_type = IR_ARG_DATA; $$->data.type = type_int; $$->data.word = $1;  $$->data.size = 4;}
            | FLOAT                          { PARSER_INIT($$); $$->lineno = yylineno; $$->arg_type = IR_ARG_DATA; $$->data.type = type_float; $$->data.word = $1;  $$->data.size = 4; }
            | ARRAY                          { PARSER_INIT($$); $$->lineno = yylineno; $$->arg_type = IR_ARG_DATA; $$->data.type = type_array; $$->data.ptr = $1.ptr; $$->data.size = $1.size; }
            | ID '(' args ')'                { PARSER_INIT($$); $$->lineno = yylineno; $$->arg_type = IR_ARG_CALL; $$->call.func = func_new($1); $$->call.args = *$3; }
            | ID                             { PARSER_INIT($$); $$->lineno = yylineno; set_arg($$, $1); }
            ;

args        : args ',' expr                  { $$ = $1; list_add($$, $3); }
            | expr                           { PARSER_INIT($$); list_add($$, $1); }
            ;
%%
