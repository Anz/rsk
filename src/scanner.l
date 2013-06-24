%option noyywrap

%{


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"

%}

id          [A-Za-z_][A-Za-z0-9_]*
sign        [+-]
int         {sign}?(0|[1-9][0-9]*)
float       {int}\.[0-9]+(e{sign}?[0-9]+)?
str         '[^']*'

%%

[ \t]       ;
[\n]        { yylineno++; }
{float}     { float val = (float)atof(yytext); memcpy(&yylval.word, &val, 4); return FLOAT; }
{int}       { yylval.word = atoi(yytext); return INT; }
{id}        { yylval.str = strdup(yytext); return ID; }
{str}       { yylval.data.size = strlen(yytext); yytext[yylval.data.size-1]  = 0; yylval.data.ptr = strdup(&yytext[1]);  return ARRAY; }
.           { yylval.str = strdup(yytext); return yytext[0]; }

%%
