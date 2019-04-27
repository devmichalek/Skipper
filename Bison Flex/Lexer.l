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

QWORD [a-zA-Z0-9]+
WORD {QWORD}|(\"[a-zA-Z0-9 ]+\")
COMMAND \!{QWORD}
OPTION (\-{QWORD})|(\-\-{QWORD})
REGEX [a-zA-Z0-9\@\#\$\%\^\&\*\.\,\(\)\?\/\\\;\:\{\}\[\]\+\=\~\-\_]+
REDIR (\<)|(\<\<)|(\>)|(\>\>)
FILE ({QWORD}\.{QWORD})|(\"{QWORD}\.{QWORD}\")
WHITESPACE [\r\t ]+
COMMENT \#.*\n

%%	/*----- Definition section -----*/
[\n]		{
				return '\n';
			}

{WHITESPACE}	{
					/*empty*/
				}

{COMMENT}	{
				/*empty*/
			}

{COMMAND}	{
				if (!interpreter.m_bError) {
					std::string error_msg = "";
					if (!interpreter.m_svSwitches.add<Subtype::COMMAND>(std::string(yytext), error_msg)) {
						interpreter.m_bError = true;
						interpreter.m_sCaughtMsg = error_msg;
					}
				}
			}

{OPTION}	{
				if (!interpreter.m_bError) {
					std::string error_msg = "";
					if (!interpreter.m_svSwitches.add<Subtype::OPTION>(std::string(yytext), error_msg)) {
						interpreter.m_bError = true;
						interpreter.m_sCaughtMsg = error_msg;
					}
				}
			}

{REGEX}		{
				if (!interpreter.m_bError) {
					std::string error_msg = "";
					if (!interpreter.m_svSwitches.add<Subtype::REGEX>(std::string(yytext), error_msg)) {
						interpreter.m_bError = true;
						interpreter.m_sCaughtMsg = error_msg;
					}
				}
			}

{REDIR}		{
				if (!interpreter.m_bError) {
					std::string error_msg = "";
					if (!interpreter.m_svSwitches.add<Subtype::REDIRECTION>(std::string(yytext), error_msg)) {
						interpreter.m_bError = true;
						interpreter.m_sCaughtMsg = error_msg;
					}
				}
			}

{WORD}		{
				if (!interpreter.m_bError) {
					std::string error_msg = "";
					if (!interpreter.m_svSwitches.add<Subtype::WORD>(std::string(yytext), error_msg)) {
						interpreter.m_bError = true;
						interpreter.m_sCaughtMsg = error_msg;
					}
				}
			}

{FILE}		{
				if (!interpreter.m_bError) {
					std::string error_msg = "";
					if (!interpreter.m_svSwitches.add<Subtype::FILE>(std::string(yytext), error_msg)) {
						interpreter.m_bError = true;
						interpreter.m_sCaughtMsg = error_msg;
					}
				}
			}

%%	/*----- User code section -----*/

// Syntax analyzer (parser).
void Interpreter::parse(const char* msg)
{
	yy_scan_string(msg);
	yyparse();
}