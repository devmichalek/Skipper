#include "cmd_echo.h"
#include "cmd_regex.h"
#include "Console.h"
#include <filesystem>

Command_Echo::Command_Echo(std::vector<std::string> options) : Command(options, Handler::CMD_ECHO)
{
	m_bEmpty = false;
	m_bHelp = false;
	m_sText = "";
}

bool Command_Echo::parse(const char* filename, int &line)
{
	if (m_options.empty())
	{
		m_bEmpty = true;
		m_sText = "\n";
	}

	for (auto &it : m_options)
	{
		if (it == "-h" || it == "--help")
		{
			m_bHelp = true;
			break;
		}
		else
			m_sText += it + "\n";
	}

	return true;
}

int Command_Echo::run()
{
	if (m_bHelp)
		output(help());
	else
		output(m_sText);

	return 0;
}