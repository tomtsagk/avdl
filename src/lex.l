%{
#include <stdio.h>
#include "yacc.tab.h"
#include "symtable.h"
int linenum = 1;

// include
#define MAX_INCLUDE_DEPTH 10
YY_BUFFER_STATE include_stack[MAX_INCLUDE_DEPTH];
int include_stack_ptr = 0;

// used to parse floats
float parsing_float = 0;

extern char included_files[10][100];
extern int included_files_num;

extern char buffer[];
extern char *includePath;

%}

/*
 * how do tokens look like
 */
number -?[0-9]+
float  -?[0-9]+\.[0-9]+
symbol [-=+<>*/%&|a-zA-Z][-=+<>*/&|a-zA-Z0-9_]*
char   \'.\'
string \"[^"]*\"

%option noyywrap

%x incl

%%

 /* when including other files, parse them first */
\(include		BEGIN(incl);
<incl>[ \t"]* /* ignore whitespace */
<incl>[^ \t\n]+[ \t]*\"[ \t]*\)	{ /* the filename of the included file */

	// get the filename removing the trailing `\")` from the syntax
	char *end = yytext;
	while (end[0] != '"') { end++; }
	end[0] = '\0';

	// make sure there aren't too many includes
	if ( include_stack_ptr >= MAX_INCLUDE_DEPTH ) {
		fprintf( stderr, "Includes nested too deeply\n" );
		exit(1);
	}

	printf("found include: %d\n", include_stack_ptr);

	// save current buffer and advance pointer
	include_stack[include_stack_ptr++] =
		YY_CURRENT_BUFFER;

	// custom include path
	if (includePath) {
		strcpy(buffer, includePath);
		strcat(buffer, "/");
		strcat(buffer, yytext);
	}
	// by default, current directory
	else {
		strcpy(buffer, yytext);
	}
	printf("input header: %s\n", buffer);

	// open new file
	yyin = fopen( buffer, "r" );
	if (!yyin) {
		fprintf( stderr, "avdl: failed to open included file: %s: %s\n", buffer, strerror(errno) );
		exit(1);
	}

	// ignore duplicates
	int isAlreadyIncluded = 0;
	for (int i = 0; i < included_files_num; i++) {
		if (strcmp(included_files[i], buffer) == 0) {
			isAlreadyIncluded = 1;
			break;
		}
	}

	yylval = symtable_insert(yytext, DD_CONSTANT_INCLUDE);
	if (!isAlreadyIncluded) {
		if (included_files_num >= 10) {
			printf("max number of included files reached\n");
			exit(-1);
		}
		strcpy(included_files[included_files_num], buffer);
		included_files_num++;
	}

	// switch buffer to new file
	yy_switch_to_buffer( yy_create_buffer( yyin, YY_BUF_SIZE ) );

	// normal parse
	BEGIN(INITIAL);

	return DD_CONSTANT_INCLUDE;
}

<<EOF>> {
	// ending initial file
	if ( --include_stack_ptr < 0 ) {
		yyterminate();
	}
	// ending included file, resume previous file
	else {
		printf("finished included file\n");
		yy_delete_buffer( YY_CURRENT_BUFFER );
		yy_switch_to_buffer( include_stack[include_stack_ptr] );
	}
}

 /* ignore comments and whitespace*/
#.*		;
[ \t]		;

 /* constants and variables */
{number} { yylval = atoi(yytext); return DD_CONSTANT_NUMBER; }
{float}  { yylval = 0; parsing_float = atof(yytext); return DD_CONSTANT_FLOAT; }
{symbol} { yylval = symtable_insert(yytext, DD_ZERO); return DD_CONSTANT_SYMBOL; }
{char}   { yylval = yytext[1]; return DD_CONSTANT_NUMBER; }

{string} {
	/* chop of the quotes from strings
	 * add it to symbol table
	 */
	char *temp = yytext +1;
	yytext[strlen(yytext) -1] = '\0';
	yylval = symtable_insert(temp, DD_CONSTANT_STRING);
	return DD_CONSTANT_STRING;
}

 /* count lines */
\n		linenum++;

 /* pass-through symbols */
.	return (int) yytext[0];

%%
