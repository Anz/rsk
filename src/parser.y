%{

#include "elf.h"
#include "x86.h"
#include "ir.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern FILE * yyin;
extern int yylex();
extern int yylineno;

struct func* func_cur = NULL;
struct func* func_first = NULL;

typedef struct parameter {
   char* name;
   struct parameter* next;
} parameter_t;

parameter_t* parameters = NULL;

int function_index = 0;

void yyerror(const char *str) {
   fprintf(stderr,"parser error on line %i: %s\n", yylineno,str);
   exit(1);
}

struct func* func_get(char* name) {
   for (struct func* f = func_first; f != NULL; f = f->next) {
      if (strcmp(f->name, name) == 0) {
         return f;
      }
   }
   
   return NULL;
}

struct func* func_new(char* name) {
   struct func* f = func_get(name);
   
   if (f == NULL) {
      f = malloc(sizeof(struct func));
      memset(f, 0, sizeof(f));
      f->name = name;
      f->next = func_first;
      func_first = f;
   }
   
   return f;
}

struct func* symbol_add(char* name) {
   struct func* f = func_get(name);
   
   if (f == NULL) {
      f = malloc(sizeof(struct func));
      memset(f, 0, sizeof(f));
      f->name = name;
      f->next = func_first;
      func_first = f;
   }
   
   parameters = NULL;
   func_cur = f;
   
   return f;
}

void parameter_add(char* name) {
   // new
   {
      func_cur->args++;
   }

   // old
   {
      parameter_t* parameter = malloc(sizeof(parameter_t));
      parameter->name = (char*)strdup(name);
      parameter->next = parameters;   
      parameters = parameter;
   }
}

int parameter_get(char* name) {
   int index = 0;
   for (parameter_t* p = parameters; p != NULL; p = p->next) {
      if (strcmp(p->name, name) == 0) {
         return func_cur->args - index - 1;
      }
      index++;
   }
      
   char error[512];
   sprintf(error, "parameter '%s' not defined", name); 
   yyerror(error);
}

int yyparse ();

struct func* parse(FILE* in) {
   // parsing
   yyin = in;
   yyparse();
   
   return func_first;
}

%}

%left  '+'  '-'
%left  '*'  '/'

%union {   
   char* str;
   int integer;
   struct value_t {
      char* data;
      size_t size;
   } value;
}

%token ID INT VAL

%type <str> ID
%type <integer> INT
%type <value> VAL

%%

line        : line definition
            | definition
            ;

definition  : name parameters '=' expr        {  }
            ;
            
name        : ID                              { symbol_add(strdup($1)); }
            ;

parameters  : '(' parameter ')'
            |
            ;

parameter   : parameter ',' ID   { parameter_add(strdup($3)); }
            | ID                 { parameter_add(strdup($1)); }
            ;
            
expr        : expr '+' term      { ir_call(func_cur, func_new("int_add")); }
            | expr '-' term      { ir_call(func_cur, func_new("int_sub")); }
            | term               { }
            ;

term        : term '*' factor    { ir_call(func_cur, func_new("int_mul")); }
            | term '/' factor    { ir_call(func_cur, func_new("int_div"));  }
            | factor
            ;

factor      : INT                { ir_loadv(func_cur, $1); }
            | ID '(' args ')'    { ir_call(func_cur, func_new(strdup($1))); }
            | ID                 { ir_loadp(func_cur, parameter_get(strdup($1))); }
            ;

args        : args ',' factor
            | factor
            ;
%%
