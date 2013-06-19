%{

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

struct ir_param* param_cur = NULL;

struct ir_type* type_int = NULL;
struct ir_type* type_float = NULL;
struct ir_type* type_array = NULL;

void yyerror(const char *str) {
   fprintf(stderr,"parser error on line %i: %s\n", yylineno,str);
   exit(1);
}

struct ir_param* parameter_get(struct ir_param* param, char* name) {
   int index = 0;
   for (struct ir_param* p = param; p != NULL; p = p->next) {
      if (strcmp(p->name, name) == 0) {
         return p;
      }
      index++;
   }
      
   char error[512];
   sprintf(error, "parameter '%s' not defined", name); 
   yyerror(error);
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
   struct ir_param* param;
   struct ir_arg* arg;
}

%token ID INT FLOAT ARRAY

%type <func> line definition
%type <param> parameters parameter
%type <arg> expr term factor args
%type <str> ID '+' '-' '*' '/'
%type <word> INT FLOAT
%type <data> ARRAY

%%

line        : line definition
            | definition
            ;

definition  : ID parameters '=' expr         { $$ = func_new($1); $$->param = $2; $$->value = $4; }
            ;

parameters  : '(' parameter ')'              { $$ = param_cur; }
            |                                { $$ = NULL; param_cur = NULL; }
            ;

parameter   : parameter ',' ID               { PARSER_INIT($$); $$->name = strdup($3); $1->next = $$; $$->index = $1->index + 1; }
            | ID                             { PARSER_INIT($$); $$->name = strdup($1); $$->index = 0; param_cur = $$; }
            ;
            
expr        : expr '+' term                  { PARSER_INIT($$); $$->arg_type = IR_ARG_CALL; $$->type = type_int; $$->call.func = func_new($2); $$->call.arg = $1; $1->next = $3; }
            | expr '-' term                  { PARSER_INIT($$); $$->arg_type = IR_ARG_CALL; $$->type = type_int; $$->call.func = func_new($2); $$->call.arg = $1; $1->next = $3; }
            | term
            ;

term        : term '*' factor                { PARSER_INIT($$); $$->arg_type = IR_ARG_CALL; $$->type = type_int; $$->call.func = func_new($2); $$->call.arg = $1; $1->next = $3; }
            | term '/' factor                { PARSER_INIT($$); $$->arg_type = IR_ARG_CALL; $$->type = type_int; $$->call.func = func_new($2); $$->call.arg = $1; $1->next = $3; }
            | factor
            ;

factor      : INT                            { PARSER_INIT($$); $$->arg_type = IR_ARG_WORD; $$->type = type_int; $$->word = $1; }
            | FLOAT                          { PARSER_INIT($$); $$->arg_type = IR_ARG_WORD; $$->type = type_float; $$->word = $1; }
            | ARRAY                          { PARSER_INIT($$); $$->arg_type = IR_ARG_DATA; $$->type = type_array; $$->data.ptr = $1.ptr; $$->data.size = $1.size; }
            | ID '(' args ')'                { PARSER_INIT($$); $$->arg_type = IR_ARG_CALL; $$->type = type_int;$$->call.func = func_new(strdup($1)); $$->call.arg = $3; }
            | ID                             { PARSER_INIT($$); $$->arg_type = IR_ARG_PARAM; $$->type = type_int; $$->param = parameter_get(param_cur, $1); }
            ;

args        : factor ',' args                { $$ = $1; $$->next = $3; }
            | factor
            ;
%%
