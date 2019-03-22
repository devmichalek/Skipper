#include "interpreter.h"

Interpreter::Interpreter()
{
	bExit = false;
	iHelp = 0;
}

void Interpreter::exit()
{
	printf("Terminating...\n");
	bExit = true;
}

void Interpreter::help()
{
	++iHelp;
	printf("\nSyntax:\n");
	printf("  [] - optional\n");
	printf("  <> - expression\n");
	printf("\nCommands:\n");
	printf("  list\n\t[-h --help]\n\t[-d --directory <directory name>]\n\t[-r --recursive]\n\t[<regular expression>]\n");
	printf("  regex\n\t[-h --help]\n\t[-m --mode]\n\t[-s --set <new mode>]\n\t[-l --list]\n");
	printf("  remove\n\t[-h --help]\n\t[-d --directory <directory name>]\n\t[-r --recursive]\n\t[-f --force]\n\t<regular expression>\n");
}

void Interpreter::parse(std::string* msg)
{
	std::string &ref = (*msg);
	for (;;)
	{
		if (ref[0] == 'l')
		{
			if (ref.substr(1) == "ist")
			{	// list

			}
		}
		else if (ref[0] == 'r')
		{
			if (ref[1] == 'e')
			{
				if (ref.substr(2) == "gex")
				{	// regex

				}
				else if (ref.substr(2) == "move")
				{	// remove

				}
			}
		}

		break;
	}

	delete msg;
}