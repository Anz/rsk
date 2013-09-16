%{

#include "map.h"
#include "elf.h"
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

void parse(char** files, int num_of_files, struct map* f) {
   file_names = files;
   number_of_files = num_of_files;

   // init structures
   funcs = f;

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
         
cmp      : cmp '=' expr                   { $$ = ir_arg_op(func_new($2)->name, $1, $3, yylineno); }
         | cmp '!' '=' expr               { $$ = ir_arg_op(func_new("!=")->name, $1, $4, yylineno); }
         | cmp '<' '=' expr               { $$ = ir_arg_op(func_new("<=")->name, $1, $4, yylineno); }
         | cmp '>' '=' expr               { $$ = ir_arg_op(func_new(">=")->name, $1, $4, yylineno); }
         | cmp '<' expr                   { $$ = ir_arg_op(func_new($2)->name, $1, $3, yylineno); }
         | cmp '>' expr                   { $$ = ir_arg_op(func_new($2)->name, $1, $3, yylineno); }
         | expr 
         ;
         

expr     : expr '+' term                  { $$ = ir_arg_op(func_new($2)->name, $1, $3, yylineno); }
         | expr '-' term                  { $$ = ir_arg_op(func_new($2)->name, $1, $3, yylineno); }
         | term
         ;

term     : term '*' factor                { $$ = ir_arg_op(func_new($2)->name, $1, $3, yylineno); }
         | term '/' factor                { $$ = ir_arg_op(func_new($2)->name, $1, $3, yylineno); }
         | factor
         ;

factor   : '(' cmp ')'                    { $$ = $2; }
         | INT                            { $$ = ir_arg_word($1, ir_type("int"), yylineno); }
         | FLOAT                          { $$ = ir_arg_word($1, ir_type("float"), yylineno); }
         | ARRAY                          { $$ = ir_arg_data($1.ptr, $1.size, ir_type("array"), yylineno); }
         | ID '(' args ')'                { $$ = ir_arg_call(func_cur, $1, $3, yylineno); struct ir_func* f = map_get(funcs, $1, strlen($1)+1); if (f) f->ref++; }
         | ID                             { $$ = ir_arg_call(func_cur, $1, NULL, yylineno); }
         ;

args     : args ',' cmp                   { $$ = $1; list_add($$, $3); }
         | cmp                            { PARSER_INIT($$); list_add($$, $1); }
         ;
%%
