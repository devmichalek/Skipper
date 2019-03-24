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
				if (it == "-h")			{ m_bHelp = true; break; }
				else if (it == "-m")	{ m_bMode = true; }
				else if (it == "-l")	{ m_bList = true; }
				else if (it == "-s")	{ m_bSetMode = true; }
			}
		}
		else
		{
			if (!m_bSetMode)
			{
				printf("Error: too little arguments for 'regex' command\n");
				return false;
			}
			else if (m_bSetMode && !m_sSetMode.empty())
			{
				printf("Error: too many arguments for 'regex' command\n");
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
		printf("Error: regex command must at least contain one argument\n");
		return 1;
	}
	else if (m_bHelp)
	{
		printf("\nDefinition:\n");
		printf("\tregex - gives regex information, changes regex behaviour\n");
		printf("\nSyntax:\n");
		printf("\t[-h --help] - prints help\n");
		printf("\t[-m --mode] - prints current regex mode\n");
		printf("\t[-l --list] - prints available regex mode\n");
		printf("\t[-s --set <new mode>] - sets new regex mode\n");
		printf("\n");
	}
	else
	{
		if (m_bMode)
		{
			std::string str = getModeString(m_iMode);
			printf("Current regex expression flavor is %s\n\n", str.c_str());
		}
		else if (m_bList)
		{
			std::string str;
			printf("Available modes are:\n");
			for (int i = 1; i <= std::regex::flag_type::egrep; i *= 2)
			{
				str = getModeString(i);
				printf("%s\n", str.c_str());
			}
			printf("\n");
		}
		else if (m_bSetMode)
		{
			if (m_sSetMode.empty())
			{
				printf("Error: new regex mode is not specified, missing <new mode> expression\n");
				return 1;
			}

			int iRes = getModeByString(m_sSetMode);
			if (iRes < 0)
			{
				printf("Error: new regex mode is invalid\n");
				return 1;
			}

			m_iMode = (std::regex::flag_type)iRes;
		}
		else {} // error
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