/*----- Prolog section -----*/
%option noyywrap
%{
	#pragma warning (disable: 4005)
	#include <iostream>
	#include "parser.h"
	#include "ConsoleInterpreter.h"
	ConsoleInterpreter consoleInterpreter;
%}

INTEGERS [0-9]+
FLOATS [0-9]*\.[0-9]+
OPTION (\"[a-zA-Z ]+\")|([a-zA-Z]+)|(\-[a-zA-Z]+)|(\-\-[a-zA-Z]+)
REGEX [a-zA-Z0-9\!\@\#\$\%\^\&\*\.\,\(\)\<\>\?\/\\\;\:\'\"\{\}\[\]\+\=\~\-\_]+
WHITESPACE [\n\r \t]
COMMENT \#.*
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
"!exit"{WHITESPACE}	{consoleInterpreter.exit(); yyterminate();}
"!help"{WHITESPACE}	{consoleInterpreter.help();}

%%	/*----- User code section -----*/
