%option noyywrap

%{


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"

char* sc_string(char* str) {
   /*int len = strlen(s);
   int shift = -1;
   s[len-1] = '\0';
   for (int i = 1; i < len; i++) {
      s[i+shift] = s[i];
      
      if (s[i] == '\\' && i+1 < len) {
         switch (s[i+1]) {
            case 'n':
               s[i+shift] = '\n';
               i++;
               shift--;
               break;
         }
      }
   }
   
   return s;*/
   
   int len = strlen(str);
   char* data = malloc(len - 2 + 4); // strlen(str) - 2x appro + 4 bytes length
   
   *((int*)data) = len - 2;
   
   int di = 4;
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
