#include <limits>
#include <iostream>
#include "parser.h"
#include "ConsoleInterpreter.h"

// Syntax analyzer (parser).
extern int yyparse();
extern int yylex();
extern FILE* yyin;
extern ConsoleInterpreter consoleInterpreter;

int main(int argc, char** argv)
{
	if (argc > 1)
	{	// Reading From File.
		FILE* fbuffer = fopen(argv[1], "r");
		if (fbuffer)
		{
			yyin = fbuffer;
			while (yylex());
		}
		else
			std::cout << "Cannot open " << argv[1] << std::endl;
	}
	else
	{	// Standard Input.
		for (;;)
		{
			yyparse();
			if (consoleInterpreter.bExit)
				break;
		}
	}

	if (!consoleInterpreter.bExit)
	{	// If terminated not by user.
		std::cout << "Press Enter to Continue...";
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	
	return 0;
}