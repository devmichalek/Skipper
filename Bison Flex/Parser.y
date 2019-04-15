/*----- Prolog section -----*/
/*Create parser.h file.*/
%defines "Generated/Parser.h"
%{
	#pragma warning (disable: 4005)
	#include <string>
	#include "Interpreter.h"
	extern Interpreter interpreter;

	// This function will be generated by flex.
	extern int yylex();		// Lexical analyzer.

	// Not generated files.
	//extern void yyerror(const char*);
	void yyerror(const char* msg)
	{
		interpreter.m_bError = true;
		printf("Error: %s", msg);
	}
%}

%union
{
	int ival;
	float fval;
	char* csval;
	std::string* sval;
}

%token <ival> INT
%token <fval> FLOAT
%token <sval> STRING
%token <csval> OPTSTR
%token <csval> CMDSTR
%token <csval> REGSTR
%token <csval> REDSTR
%type<ival> int_exp
%type<fval> float_exp
%type<sval> cmd_exp
%type<sval> cmd_exp_r
%type<sval> cmd_exp_rr

%left '-' '+'
%left '*' '/'


%%	/*----- Grammar section (rules) -----*/

input: /*empty*/
	| input line
	;

line: '\n'
	| int_exp '\n'		{std::cout << "=" << $1 << std::endl;}
	| float_exp '\n'	{std::cout << "=" << $1 << std::endl;}
	| cmd_exp_rr '\n'	{interpreter.analyze($1);}
	| cmd_exp_r '\n'	{interpreter.analyze($1);}
	| cmd_exp '\n'		{interpreter.analyze($1);}
	;

int_exp: INT				{$$ = $1;}
	| int_exp '+' int_exp	{$$ = $1 + $3;}
	| int_exp '-' int_exp	{$$ = $1 - $3;}
	| int_exp '*' int_exp	{$$ = $1 * $3;}
	| int_exp '/' int_exp	{$$ = $1 / $3;}
	| '(' int_exp ')'		{$$ = $2;}
	;

float_exp: FLOAT				{$$ = $1;}
	| float_exp '+' float_exp	{$$ = $1 + $3;}
	| float_exp '-' float_exp	{$$ = $1 - $3;}
	| float_exp '*' float_exp	{$$ = $1 * $3;}
	| float_exp '/' float_exp	{$$ = $1 / $3;}
	| '(' float_exp ')'			{$$ = $2;}
	;

cmd_exp_rr: cmd_exp REDSTR	{
								std::string* str = $1;
								(*str) += " " + std::string($2);
								$$ = str;
							}
	| cmd_exp_r REDSTR		{
								std::string* str = $1;
								(*str) += " " + std::string($2);
								$$ = str;
							}
	;

cmd_exp_r: cmd_exp REGSTR	{
								std::string* str = $1;
								(*str) += " " + std::string($2);
								$$ = str;
							}
	| cmd_exp REGSTR REGSTR {
								std::string* str = $1;
								(*str) += " " + std::string($2) + " " + std::string($3);
								$$ = str;
							}
	;

cmd_exp: CMDSTR	{
					std::string* str = new std::string($1);
					$$ = str;
				}
	| CMDSTR OPTSTR	{
						std::string* str = new std::string($1);
						(*str) += " " + std::string($2);
						$$ = str;
					}
	| CMDSTR OPTSTR	OPTSTR {
								std::string* str = new std::string($1);
								(*str) += " " + std::string($2);
								(*str) += " " + std::string($3);
								$$ = str;
							}
	| CMDSTR OPTSTR	OPTSTR OPTSTR	{
										std::string* str = new std::string($1);
										(*str) += " " + std::string($2);
										(*str) += " " + std::string($3);
										(*str) += " " + std::string($4);
										$$ = str;
									}
	| CMDSTR OPTSTR	OPTSTR OPTSTR OPTSTR	{
												std::string* str = new std::string($1);
												(*str) += " " + std::string($2);
												(*str) += " " + std::string($3);
												(*str) += " " + std::string($4);
												(*str) += " " + std::string($5);
												$$ = str;
											}
	| CMDSTR OPTSTR	OPTSTR OPTSTR OPTSTR OPTSTR {
													std::string* str = new std::string($1);
													(*str) += " " + std::string($2);
													(*str) += " " + std::string($3);
													(*str) += " " + std::string($4);
													(*str) += " " + std::string($5);
													(*str) += " " + std::string($6);
													$$ = str;
												}
	;

%%	/*----- User code section -----*/
