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

char** file_names = NULL;
int number_of_files = 0;
int index_of_file = 0;


struct map* funcs = NULL;

struct ir_func* func_cur = NULL;

struct ir_type* type_bool = NULL;
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

static struct ir_func* func_prototype(char* name, int args, struct ir_type* res) {   
   // setup functions
   struct ir_func* f = func_new(name);
   f->type = res;
   
   for (int i = 0; i < args; i++) {
      struct ir_param* param = malloc(sizeof(*param));
      memset(param, 0, sizeof(*param));
      char buf[100];
      sprintf(buf, "arg%i", i);
      param->name = strdup(buf);
      
      map_set(&f->params, param->name, strlen(param->name)+1, param);
   }
   
   return f;
}

void parse(char** files, int num_of_files, struct map* f) {
   file_names = files;
   number_of_files = num_of_files;

   // init structures
   funcs = f;

   // setup types
   type_bool = malloc(sizeof(*type_bool));
   type_bool->name = "bool";
   map_init(&type_bool->ops);
   map_set(&type_bool->ops, "=", 2, func_prototype("int=", 2, type_bool));
   map_set(&type_bool->ops, "<", 2, func_prototype("int<", 2, type_bool));
   map_set(&type_bool->ops, ">", 2, func_prototype("int>", 2, type_bool));
   map_set(&type_bool->ops, "<=", 3, func_prototype("int<=", 2, type_bool));
   map_set(&type_bool->ops, ">=", 3, func_prototype("int>=", 2, type_bool));
   
   
   type_int = malloc(sizeof(*type_int));
   type_int->name = "int";
   map_init(&type_int->ops);
   map_set(&type_int->ops, "+", 2, func_prototype("int+", 2, type_int));
   map_set(&type_int->ops, "-", 2, func_prototype("int-", 2, type_int));
   map_set(&type_int->ops, "*", 2, func_prototype("int*", 2, type_int));
   map_set(&type_int->ops, "/", 2, func_prototype("int/", 2, type_int));
   map_set(&type_int->ops, "=", 2, func_prototype("int=", 2, type_bool));
   map_set(&type_int->ops, "<", 2, func_prototype("int<", 2, type_bool));
   map_set(&type_int->ops, ">", 2, func_prototype("int>", 2, type_bool));
   map_set(&type_int->ops, "<=", 3, func_prototype("int<=", 2, type_bool));
   map_set(&type_int->ops, ">=", 3, func_prototype("int>=", 2, type_bool));
   
   type_float = malloc(sizeof(*type_float));
   type_float->name = "float";
   map_init(&type_float->ops);
   map_set(&type_float->ops, "+", 2, func_prototype("float+", 2, type_float));
   
   type_array = malloc(sizeof(*type_array));
   type_array->name = "array";
   map_init(&type_array->ops);
   map_set(&type_array->ops, "+", 2, func_prototype("array+", 2, type_array));
   map_set(&type_array->ops, "=", 2, func_prototype("array=", 2, type_bool));
   map_set(&type_array->ops, "!=", 3, func_prototype("array!=", 2, type_bool));
   
   // setup functions
   func_prototype("+", 2, NULL);
   func_prototype("-", 2, NULL);
   func_prototype("*", 2, NULL);
   func_prototype("/", 2, NULL);
   func_prototype("=", 2, NULL);
   func_prototype("!=", 2, NULL);
   func_prototype("<", 2, NULL);
   func_prototype(">", 2, NULL);
   func_prototype("<=", 2, NULL);
   func_prototype(">=", 2, NULL);
   func_prototype("stdin", 1, type_array);

   // parsing
   yyin = fopen(file_names[index_of_file], "r");
   if (yyin == NULL) {
      printf("could not open file %s", file_names[index_of_file]);
      exit(1);
   }
   yyparse();
}

int yywrap() {
   fclose(yyin);
   
   index_of_file++;
   if (index_of_file >= number_of_files) {
      return 1;
   }
   
   yyin = fopen(file_names[index_of_file], "r");
   if (yyin == NULL) {
      printf("could not open file %s", file_names[index_of_file]);
      exit(1);
   }
   return 0;
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
%type <arg> cmp expr term factor
%type <list> args
%type <str> ID '=' '<' '>' '+' '-' '*' '/'
%type <word> INT FLOAT
%type <data> ARRAY

%%
         
line     : line def
         | def
         ;

def      : var '(' param ')' '=' case
         | var '(' param ')' '=' cmp '\n' { list_add(&$1->cases, ir_func_case(NULL, $6, yylineno)); }
         | var '=' cmp '\n'               { list_add(&$1->cases, ir_func_case(NULL, $3, yylineno)); }
         | '\n'
         ;
            
var      : ID                             { $$ = func_cur = func_new($1); $$->lineno = yylineno; $$->file = file_names[index_of_file]; map_set(funcs, $$->name, strlen($$->name) + 1, $$); }
         ;
            
param    : param ',' ID                   { ir_func_param(func_cur, $3, yylineno); }
         | ID                             { ir_func_param(func_cur, $1, yylineno); }
         ;
      
case     : case '{' cmp ',' cmp '\n'      { list_add(&func_cur->cases, ir_func_case($5, $3, yylineno)); }
         | '{' cmp ',' cmp '\n'           { list_add(&func_cur->cases, ir_func_case($4, $2, yylineno)); }
         | '{' cmp '\n'                   { list_add(&func_cur->cases, ir_func_case(NULL, $2, yylineno)); }
         ;
         
cmp      : cmp '=' expr                   { $$ = ir_arg_op(func_new($2), $1, $3, yylineno); }
         | cmp '!' '=' expr               { $$ = ir_arg_op(func_new("!="), $1, $4, yylineno); }
         | cmp '<' '=' expr               { $$ = ir_arg_op(func_new("<="), $1, $4, yylineno); }
         | cmp '>' '=' expr               { $$ = ir_arg_op(func_new(">="), $1, $4, yylineno); }
         | cmp '<' expr                   { $$ = ir_arg_op(func_new($2), $1, $3, yylineno); }
         | cmp '>' expr                   { $$ = ir_arg_op(func_new($2), $1, $3, yylineno); }
         | expr 
         ;
         

expr     : expr '+' term                  { $$ = ir_arg_op(func_new($2), $1, $3, yylineno); }
         | expr '-' term                  { $$ = ir_arg_op(func_new($2), $1, $3, yylineno); }
         | term
         ;

term     : term '*' factor                { $$ = ir_arg_op(func_new($2), $1, $3, yylineno); }
         | term '/' factor                { $$ = ir_arg_op(func_new($2), $1, $3, yylineno); }
         | factor
         ;

factor   : '(' cmp ')'                    { $$ = $2; }
         | INT                            { $$ = ir_arg_word($1, type_int, yylineno); }
         | FLOAT                          { $$ = ir_arg_word($1, type_float, yylineno); }
         | ARRAY                          { $$ = ir_arg_data($1.ptr, $1.size, type_array, yylineno); }
         | ID '(' args ')'                { $$ = ir_arg_call(funcs, func_cur, $1, $3, yylineno); }
         | ID                             { $$ = ir_arg_call(funcs, func_cur, $1, NULL, yylineno); }
         ;

args     : args ',' cmp                   { $$ = $1; list_add($$, $3); }
         | cmp                            { PARSER_INIT($$); list_add($$, $1); }
         ;
%%
