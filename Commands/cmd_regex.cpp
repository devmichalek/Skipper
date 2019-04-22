#include "cmd_regex.h"
#include "Console.h"

std::regex::flag_type Command_Regex::m_iMode = std::regex::flag_type::ECMAScript;

Command_Regex::Command_Regex(std::vector<std::string> options) : Command(options, Handler::CMD_REGEX)
{
	m_bEmpty = false;
	m_bHelp = false;
	m_bMode = false;
	m_bSetMode = false;
	m_bList = false;
	m_sSetMode = "";
}

bool Command_Regex::parse(const char* filename, int &line)
{
	if (m_options.empty())
		m_bEmpty = true;

	for (auto &it : m_options)
	{
		if (it[0] == '-')
		{
			if (it[1] == '-')
			{
				if (it == "--help")			{ m_bHelp = true; break; }
				else if (it == "--mode")	{ m_bMode = true; }
				else if (it == "--list")	{ m_bList = true; }
				else if (it == "--set")		{ m_bSetMode = true; }
			}
			else
			{
				if (it == "-h")						{ m_bHelp = true; break; }
				else if (it == "-m" && !m_bMode)	{ m_bMode = true; }
				else if (it == "-l" && !m_bList)	{ m_bList = true; }
				else if (it == "-s" && !m_bSetMode)	{ m_bSetMode = true; }
				else
				{
					std::string res = "Switch " + it + " is either already specified or not valid for the 'regex' command";
					PrintError(filename, line, res.c_str());
					return false;
				}
			}
		}
		else
		{
			if (!m_bSetMode)
			{
				PrintError(filename, line, "Missing --set switch for the 'regex' command");
				return false;
			}
			else if (m_bSetMode && !m_sSetMode.empty())
			{
				PrintError(filename, line, "Too many arguments for the 'regex' command");
				return false;
			}
			else
				m_sSetMode = it;
		}
	}

	if (m_bEmpty) {
		PrintError(filename, line, "'regex' command must at least contain one argument");
		return false;
	}

	if (m_bSetMode)
	{
		if (m_sSetMode.empty()) {
			PrintError(filename, line, "<new mode> for --set switch is not specified for the 'regex' command");
			return false;
		}

		int iRes = getModeByString(m_sSetMode);
		if (iRes < 0) {
			PrintError(filename, line, "<new mode> for switch --set for the 'regex' command is invalid");
			return false;
		}
	}

	return true;
}

int Command_Regex::run()
{
	if (m_bHelp)
		output(help());
	else if (m_bMode)
	{
		std::string str = getModeString(m_iMode);
		output("Current regular expression flavor is " + str + "\n\n");
	}
	else if (m_bList)
	{
		std::string str;
		output("\nAvailable modes are:\n");
		for (int i = 1; i <= std::regex::flag_type::egrep; i *= 2)
		{
			str = getModeString(i);
			output(str + "\n");
		}
		output("\n");
	}
	else if (m_bSetMode)
	{
		int iRes = getModeByString(m_sSetMode);
		m_iMode = (std::regex::flag_type)iRes;
	}

	return 0;
}

std::string Command_Regex::getModeString(const int &i)
{
	switch (i)
	{
		case std::regex::flag_type::ECMAScript:	return "ECMAScript";
		case std::regex::flag_type::basic:		return "basic";
		case std::regex::flag_type::extended:	return "extended";
		case std::regex::flag_type::grep:		return "grep";
		case std::regex::flag_type::egrep:		return "egrep";
		case std::regex::flag_type::awk:		return "awk";
	}

	return "";
}

int Command_Regex::getModeByString(const std::string &str)
{
	if (str == "ECMAScript")	return std::regex::flag_type::ECMAScript;
	if (str == "basic")			return std::regex::flag_type::basic;
	if (str == "extended")		return std::regex::flag_type::extended;
	if (str == "grep")			return std::regex::flag_type::grep;
	if (str == "egrep")			return std::regex::flag_type::egrep;
	if (str == "awk")			return std::regex::flag_type::awk;
	return -1;
}