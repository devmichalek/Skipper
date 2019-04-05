/*----- Prolog section -----*/
%option noyywrap
%{
	#pragma warning (disable: 4005)
	#include <string>
	#include "parser.h"
	#include "Interpreter.h"
	Interpreter interpreter;
%}

INTEGERS [0-9]+
FLOATS [0-9]*\.[0-9]+
OPTION (\"[a-zA-Z ]+\")|([a-zA-Z]+)|(\-[a-zA-Z]+)|(\-\-[a-zA-Z]+)
REGEX [a-zA-Z0-9\!\@\#\$\%\^\&\*\.\,\(\)\<\>\?\/\\\;\:\'\"\{\}\[\]\+\=\~\-\_]+
WHITESPACE [\n\r \t]
COMMENT \#.*\n
REDIRECTION [\>]|[\>\>]

%%	/*----- Definition section -----*/
{INTEGERS}	{yylval.ival = atoi(yytext);	return INT;}
{FLOATS}	{yylval.fval = (float)atof(yytext);	return FLOAT;}

[+*/()-]	{return yytext[0];}
[\n]		{return '\n';}
[ \t]		{/*empty*/}
{COMMENT}	{/*empty*/}
.			{/*empty*/}

{OPTION}	{yylval.csval = _strdup(yytext); return OPTSTR;}
\![a-zA-Z]+	{yylval.csval = _strdup(yytext); return CMDSTR;}
{REGEX}|({REGEX}{REDIRECTION}{REGEX})	{yylval.csval = _strdup(yytext); return REGSTR;}
"!exit"{WHITESPACE}	{interpreter.exit(); yyterminate();}

%%	/*----- User code section -----*/

// Syntax analyzer (parser).
void Interpreter::parse(const char* msg)
{
	yy_scan_string(msg);
	yyparse();
}