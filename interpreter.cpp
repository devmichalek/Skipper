#include "interpreter.h"
#include "cmd.h"
#include "cmd_list.h"
#include "cmd_regex.h"
#include "cmd_remove.h"

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
	printf("\nExamples:\n");
	printf("  list --recursive --directory \"a\\b\\c\" [a-z].txt\n");
	printf("  list -r *.php\n");
	printf("  list -rd \"foo\" [abc+]\\.js\n");
	printf("\nCommands:\n");
	printf("  list\n\t[-h --help]\n\t[-d --directory <directory name>]\n\t[-r --recursive]\n\t[<regular expression>]\n");
	printf("  regex\n\t[-h --help]\n\t[-m --mode]\n\t[-l --list]\n\t[-s --set <new mode>]\n");
	printf("  remove\n\t[-h --help]\n\t[-d --directory <directory name>]\n\t[-v --verbose]\n\t[-r --recursive]\n\t<regular expression>\n");
	printf("\n");
}

void Interpreter::parse(std::string* msg)
{
	if (bError)
	{
		if (msg)
			delete msg;
		bError = false;
		return;
	}

	if (!msg)
		return;

	std::string &ref = (*msg);
	Command* command = nullptr;

	if (ref[0] == '!')
	{
		if (ref[1] == 'l')
		{
			if (ref.substr(2, 3) == "ist")
			{	// list
				command = new Command_List(extract(ref, 6));
			}
		}
		else if (ref[1] == 'r')
		{
			if (ref[2] == 'e')
			{
				if (ref.substr(3, 3) == "gex")
				{	// regex
					command = new Command_Regex(extract(ref, 7));
				}
				else if (ref.substr(3, 4) == "move")
				{	// remove
					command = new Command_Remove(extract(ref, 8));
				}
			}
		}
	}

	if (command)
	{
		if (command->parse())
			command->run();
		delete command;
	}

	if (msg)
		delete msg;
}