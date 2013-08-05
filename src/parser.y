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

struct ir_func* func_cur = NULL;

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
      f = ir_func_init(name, -1);      
      map_set(funcs, f->name, strlen(f->name) + 1, f);
   }
   
   return f;
}

int yyparse ();

static struct ir_func* binary_op(char* name, struct ir_type* type) {
   struct ir_param* a = malloc(sizeof(*a));
   memset(a, 0, sizeof(*a));
   a->name = strdup("a");
   a->type = type;
   
   struct ir_param* b = malloc(sizeof(*b));
   memset(b, 0, sizeof(*b));
   b->name = strdup("b");
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

%type <func> var
%type <arg> expr term factor
%type <list> args
%type <str> ID '+' '-' '*' '/'
%type <word> INT FLOAT
%type <data> ARRAY

%%

line     : line def
         | def
         ;

def      : var '(' param ')' '=' expr     { $1->value = $6; }
         | var '=' expr                   { $1->value = $3; }
         ;
            
var      : ID                             { $$ = func_cur = func_new($1); $$->lineno = yylineno; map_set(funcs, $$->name, strlen($$->name) + 1, $$); }
         ;
            
param    : param ',' ID                   { ir_func_param(func_cur, $3, yylineno); }
         | ID                             { ir_func_param(func_cur, $1, yylineno); }
         ;

expr     : expr '+' term                  { PARSER_INIT($$); $$->lineno = yylineno; $$->arg_type = IR_ARG_CALL; $$->call.func = func_new($2); list_add(&$$->call.args, $1); list_add(&$$->call.args, $3); }
         | expr '-' term                  { PARSER_INIT($$); $$->lineno = yylineno; $$->arg_type = IR_ARG_CALL; $$->call.func = func_new($2); list_add(&$$->call.args, $1); list_add(&$$->call.args, $3); }
         | term
         ;

term     : term '*' factor                { PARSER_INIT($$);$$->lineno = yylineno;  $$->arg_type = IR_ARG_CALL; $$->call.func = func_new($2); list_add(&$$->call.args, $1); list_add(&$$->call.args, $3); }
         | term '/' factor                { PARSER_INIT($$);$$->lineno = yylineno;  $$->arg_type = IR_ARG_CALL; $$->call.func = func_new($2); list_add(&$$->call.args, $1); list_add(&$$->call.args, $3);   }
         | factor
         ;

factor   : INT                            { $$ = ir_arg_word($1, type_int, yylineno); }
         | FLOAT                          { $$ = ir_arg_word($1, type_float, yylineno); }
         | ARRAY                          { $$ = ir_arg_data($1.ptr, $1.size, type_array, yylineno); }
         | ID '(' args ')'                { $$ = ir_arg_call(funcs, func_cur, $1, $3, yylineno); }
         | ID                             { $$ = ir_arg_call(funcs, func_cur, $1, NULL, yylineno); }
         ;

args     : args ',' expr                  { $$ = $1; list_add($$, $3); }
         | expr                           { PARSER_INIT($$); list_add($$, $1); }
         ;
%%
