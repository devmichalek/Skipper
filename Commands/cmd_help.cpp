#include "cmd_help.h"
#include "cmd_compare.h"
#include "cmd_copy.h"
#include "cmd_include.h"
#include "cmd_list.h"
#include "cmd_move.h"
#include "cmd_regex.h"
#include "cmd_remove.h"
#include "cmd_rename.h"
#include "cmd_run.h"
#include "cmd_search.h"
#include "cmd_wait.h"
#include "cmd_wipe.h"
#include "Console.h"

Command_Help::Command_Help(std::vector<std::string> options) : Command(options, Handler::CMD_HELP)
{
	m_bEmpty = false;
	m_bHelp = false;
}

bool Command_Help::parse(const char* filename, int &line)
{
	if (m_options.empty())
		m_bEmpty = true;

	for (auto &it : m_options)
	{
		if (it[0] == '-')
		{
			if (it[1] == '-')
			{
				if (it == "--help") { m_bHelp = true; break; }
			}
			else
			{
				if (!validate(it, "h"))
				{
					std::string res = "Cannot resolve " + it + " switch for the 'help' command";
					PrintError(filename, line, res.c_str());
					return false;
				}

				if (it.find('h') != std::string::npos) { m_bHelp = true; break; }
			}
		}
		else
		{
			PrintError(filename, line, "Too many arguments for the 'help' command\n");
			return false;
		}
	}

	return true;
}

int Command_Help::run()
{
	if (m_bEmpty)
	{
		output("\nSyntax:\n");
		output("  [] - optional\n");
		output("  <> - expression\n");
		output("\nExamples:\n");
		output("  list --recursive --directory \"a\\b\\c\" [a-z]+\\.txt\n");
		output("  compare -r [0-9]+\\.txt [0-9]+\\.pat\n");
		output("  list -rd \"foo\" [abc+]\\.js\n");
		output("\nCommands (in alphabetical order):\n");
		output(Command_Compare::assist());
		output(Command_Copy::assist());
		output(Command_Help::assist());
		output(Command_Include::assist());
		output(Command_List::assist());
		output(Command_Move::assist());
		output(Command_Regex::assist());
		output(Command_Remove::assist());
		output(Command_Rename::assist());
		output(Command_Run::assist());
		output(Command_Search::assist());
		output(Command_Wait::assist());
		output(Command_Wipe::assist());
		output("\n");
	}
	else if (m_bHelp)
		output(help());

	return 0;
}