#include "cmd_help.h"
#include "cmd_include.h"
#include "cmd_list.h"
#include "cmd_regex.h"
#include "cmd_remove.h"

Command_Help::Command_Help(std::vector<std::string> options) : Command(options, Handler::CMD_HELP)
{
	m_bEmpty = false;
	m_bHelp = false;
}

bool Command_Help::parse()
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
					output("Error: cannot resolve " + it + " switch\n");
					return false;
				}

				if (it.find('h') != std::string::npos) { m_bHelp = true; break; }
			}
		}
		else
		{
			output("Error: too many arguments for 'help' command\n");
			return false;
		}
	}

	return true; // no error
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
		output("  list -r *.php\n");
		output("  list -rd \"foo\" [abc+]\\.js\n");
		output("\nCommands (in alphabetical order):\n");
		output(Command_Help::assist());
		output(Command_Include::assist());
		output(Command_List::assist());
		output(Command_Regex::assist());
		output(Command_Remove::assist());
		output("\n");
	}
	else if (m_bHelp)
		output(help());

	return 0; // no error
}