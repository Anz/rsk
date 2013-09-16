%{


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"

extern void yyterminate();

char* sc_string(char* str) {
   int len = strlen(str);
   char* data = malloc(len - 2); // strlen(str) - 2x appro
   
   int di = 0;
   for (int si = 1; si < len - 1; si++) {
      if (strncmp("\\n", &str[si], 2) == 0) {
         data[di++] = '\n';
         si++;
      } else {
         data[di++] = str[si];
      }
   }
   
   return data;
}

%}

id          [A-Za-z_][A-Za-z0-9_]*
sign        [+-]
int         {sign}?(0|[1-9][0-9]*)
float       {int}\.[0-9]+(e{sign}?[0-9]+)?
str         '[^']*'
comment     \/\/[^\n]*

%%

[ \t]       ;
[\n]        { yylineno++; return yytext[0]; }
{comment}   ;
{float}     { float val = (float)atof(yytext); memcpy(&yylval.word, &val, 4); return FLOAT; }
{int}       { yylval.word = atoi(yytext); return INT; }
{id}        { yylval.str = strdup(yytext); return ID; }
{str}       { yylval.data.ptr = sc_string(yytext); yylval.data.size = strlen(yytext) -2 + 4; return ARRAY; }
.           { yylval.str = strdup(yytext); return yytext[0]; }

%%
