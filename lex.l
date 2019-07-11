%{
#include <stdio.h>
#include "yacc.tab.h"
#include "symtable.h"
extern int yylex();
int linenum = 1;
%}

symbol [a-zA-Z][a-zA-Z0-9_]*
number [0-9]+

%option noyywrap

%%

 /* ignore comments */
#.*		;

 /* constants and variables */
{symbol}	{ yylval = symtable_insert(yytext, DD_ZERO); return DD_CONSTANT_SYMBOL; }
{number}	{ yylval = atoi(yytext); return DD_CONSTANT_NUMBER; }
\"[^"]*\"	{
	/* chop of the quotes from strings
	 * add it to symbol table
	 */
	char *temp = yytext +1;
	yytext[strlen(yytext) -1] = '\0';
	yylval = symtable_insert(temp, DD_CONSTANT_STRING);
	return DD_CONSTANT_STRING;
}

 /* pass-through symbols */
[-/*=+\.><]	{ yylval = symtable_insert(yytext, yytext[0]); return DD_CONSTANT_CHARACTER; }
[()]		{ return yytext[0]; }

 /* ignore whitespace */
[ \t]		;

 /* count lines */
\n		linenum++;

%%
