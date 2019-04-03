#include "cmd_regex.h"

std::regex::flag_type Command_Regex::m_iMode = std::regex::flag_type::ECMAScript;

Command_Regex::Command_Regex(std::vector<std::string> options) : Command(options)
{
	m_bEmpty = false;
	m_bHelp = false;
	m_bMode = false;
	m_bSetMode = false;
	m_bList = false;
	m_sSetMode = "";
}

bool Command_Regex::parse()
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
					output("Error: switch " + it + " is already specified\n");
					return false;
				}
			}
		}
		else
		{
			if (!m_bSetMode)
			{
				output("Error: too little arguments for 'regex' command\n");
				return false;
			}
			else if (m_bSetMode && !m_sSetMode.empty())
			{
				output("Error: too many arguments for 'regex' command\n");
				return false;
			}
			else
				m_sSetMode = it;
		}
	}

	return true; // no error
}

int Command_Regex::run()
{
	if (m_bEmpty)
	{
		output("Error: regex command must at least contain one argument\n");
		return 1;
	}
	else if (m_bHelp)
		output(help());
	else
	{
		if (m_bMode)
		{
			std::string str = getModeString(m_iMode);
			output("Current regex expression flavor is " + str + "\n\n");
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
			if (m_sSetMode.empty())
			{
				output("Error: new regex mode is not specified, missing <new mode> expression\n");
				return 1;
			}

			int iRes = getModeByString(m_sSetMode);
			if (iRes < 0)
			{
				output("Error: new regex mode is invalid\n");
				return 1;
			}

			m_iMode = (std::regex::flag_type)iRes;
		}
	}

	return 0; // no error
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