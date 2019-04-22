/*----- Prolog section -----*/
%option noyywrap
%{
	#pragma warning (disable: 4005)
	#include <string>
	#include "Parser.h"
	#include "Interpreter.h"
	RegularScope* globalScope = nullptr;
	Interpreter interpreter(globalScope);
%}

INTEGERS [0-9]+
FLOATS [0-9]*\.[0-9]+
OPTION (\-[a-zA-Z]+)|(\-\-[a-zA-Z]+)
REGEX [a-zA-Z0-9\@\#\$\%\^\&\*\.\,\(\)\?\/\\\;\:\'\"\{\}\[\]\+\=\~\-\_]+
WHITESPACE [\n\r \t]
COMMENT \#.*\n
REDIR [\>\<]+

%%	/*----- Definition section -----*/
{INTEGERS}	{yylval.ival = atoi(yytext);	return INT;}
{FLOATS}	{yylval.fval = (float)atof(yytext);	return FLOAT;}

[+*/()-]	{return yytext[0];}
[\n]		{return '\n';}
[ \t]		{/*empty*/}
{COMMENT}	{/*empty*/}

{OPTION}	{yylval.csval = _strdup(yytext); return OPTSTR;}
\![a-zA-Z]+	{yylval.csval = _strdup(yytext); return CMDSTR;}
{REGEX}		{yylval.csval = _strdup(yytext); return REGSTR;}
{REDIR}		{yylval.csval = _strdup(yytext); return REDSTR;}

%%	/*----- User code section -----*/

// Syntax analyzer (parser).
void Interpreter::parse(const char* msg)
{
	yy_scan_string(msg);
	yyparse();
}