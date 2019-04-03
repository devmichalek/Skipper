#include "Interpreter.h"
#include "cmd.h"
#include "cmd_help.h"
#include "cmd_include.h"
#include "cmd_list.h"
#include "cmd_regex.h"
#include "cmd_remove.h"

Interpreter::Interpreter()
{
	bExit = false;
}

void Interpreter::exit()
{
	printf("Terminating...\n");
	bExit = true;
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