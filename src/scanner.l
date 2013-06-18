%option noyywrap

%{


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parser.h"

%}


%%

[ \t]                   ;
[\n]                    { yylineno++; }
(0|[1-9][0-9]*)         { yylval.integer = atoi(yytext); return INT; }
[A-Za-z_][A-Za-z0-9_]*  { yylval.str = strdup(yytext); return ID; }
.                       { yylval.str = strdup(yytext); return yytext[0]; }

%%
