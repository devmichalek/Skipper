#include "cmd_wipe.h"
#include "cmd_regex.h"
#include <fstream>
#include <filesystem>

bool Command_Wipe::m_bChosenMode = true;

Command_Wipe::Command_Wipe(std::vector<std::string> options) : Command(options, Handler::CMD_WIPE)
{
	m_bEmpty = false;
	m_bHelp = false;
	m_bDirectory = false;
	m_bMode = false;
	m_bSet = false;
	m_bRange = false;
	m_bLine = false;
	m_bKeyword = false;
	m_bRegex = false;
	m_sDirectory = "";
	m_sKeyword = "";
	m_sRegex = "";
	m_iRangeStart = -1;
	m_iRangeEnd = -1;
	m_iLine = -1;
}

bool Command_Wipe::parse()
{
	if (m_options.empty())
		m_bEmpty = true;

	std::string line = "", range = "", set = "";

	for (auto &it : m_options)
	{
		if (it[0] == '-')
		{
			if (it[1] == '-')
			{
				if (it == "--help")				{ m_bHelp = true; break; }
				else if (it == "--directory")	{ m_bDirectory = true; }
				else if (it == "--mode")		{ m_bMode = true; }
				else if (it == "--set")			{ m_bSet = true; }
				else if (it == "--range")		{ m_bRange = true; }
				else if (it == "--line")		{ m_bLine = true; }
				else if (it == "--keyword")		{ m_bKeyword = true; }
			}
			else
			{
				if (it.size() > 2)
				{
					output("Error: cannot resolve " + it + " switch\n");
					return false;
				}

				if (it.find('h') != std::string::npos) { m_bHelp = true; break; }
				if (it.find('d') != std::string::npos) { m_bDirectory = true; }
				if (it.find('m') != std::string::npos) { m_bMode = true; }
				if (it.find('s') != std::string::npos) { m_bSet = true; }
				if (it.find('r') != std::string::npos) { m_bRange = true; }
				if (it.find('l') != std::string::npos) { m_bLine = true; }
				if (it.find('k') != std::string::npos) { m_bKeyword = true; }
			}
		}
		else
		{
			if (m_bDirectory && m_sDirectory.empty()) { m_sDirectory = it; }
			else if (m_bSet && set.empty()) { set = it; }
			else if (m_bRange && range.empty()) { range = it; }
			else if (m_bLine && line.empty()) { line = it; }
			else if (m_bKeyword && m_sKeyword.empty()) { m_sKeyword = it; }
			else if (m_sRegex.empty()) { m_bRegex = true; m_sRegex = it; }
			else
			{
				output("Error: too many arguments for 'wipe' command\n");
				return false;
			}
		}
	}

	if (m_bMode && m_bSet)
	{
		if (!range.empty() || !m_sKeyword.empty())
		{
			output("Error: too many options turned on for 'wipe' command\n");
			return false;
		}
	}

	if (m_bSet)
	{	// Evaluate 0 or 1
		if (set.size() > 1)
		{
			output("Error: argument " + set + " for --set option for the 'wipe' command is wrong\n");
			return false;
		}

		if (set[0] != '0' && set[0] != '1')
		{
			output("Error: argument for --set option can be either 1 or 0 for the 'wipe' command\n");
			return false;
		}

		m_bChosenMode = bool(int(set[0]) - 48);
	}

	// only one option possible
	if (!line.empty())
	{
		if (!range.empty() || !m_sKeyword.empty())
		{
			output("Error: too many options turned on for the 'wipe' command\n");
			return false;
		}

		for (auto &it : line)
		{
			if (!isdigit(it))
			{
				output("Error: --line option for the 'wipe' command should contain only numbers as an argument\n");
				return false;
			}
		}

		m_iLine = atoi(line.c_str());
	}
	else if (!range.empty())
	{
		if (!line.empty() || !m_sKeyword.empty())
		{
			output("Error: too many options turned on for the 'wipe' command\n");
			return false;
		}

		size_t pos = range.find(' ');
		if (pos == std::string::npos)
		{
			output("Error: --range option for the 'wipe' command is not correctly specified\n");
			return false;
		}

		std::string sStart = range.substr(0, pos);
		for (auto &it : sStart)
			if (!isdigit(it))
			{
				output("Error: --range option for the 'wipe' command should contain only numbers as an argument\n");
				return false;
			}
		m_iRangeStart = atoi(sStart.c_str());

		std::string sEnd = range.substr(pos + 1);
		for (auto &it : sEnd)
			if (!isdigit(it))
			{
				output("Error: --range option for 'wipe' command should contain only numbers as an argument\n");
				return false;
			}
		m_iRangeEnd = atoi(sEnd.c_str());
	}
	else if (!m_sKeyword.empty())
	{
		if (!line.empty() || !range.empty())
		{
			output("Error: too many options turned on for the 'wipe' command\n");
			return false;
		}
	}

	return true; // no error
}

int Command_Wipe::run()
{
	if (m_bHelp)
		output(help());
	else if (m_bEmpty)
	{
		output("Error: too little arguments for 'wipe' command\n");
		return 1;
	}
	else if (m_bMode)
	{
		output("Available modes are 1 - descending order and 0 - ascending order\n");
		output("With descending order file will be read from the top to the bottom, the second mode will do the opposite\n");
		char out[2] = { (char)(((int)m_bChosenMode) + 48), '\0'};
		output("Current mode is " + std::string(out) + "\n");
	}
	else if (!m_bSet)
	{	// if it is not --set switch
		std::filesystem::path path;
		if (m_bDirectory)
			path = m_sDirectory;
		else
			path = std::filesystem::current_path();

		std::vector<std::string> result;
		for (const auto & entry : std::filesystem::directory_iterator(path))
			result.push_back(entry.path().string());

		if (result.empty())
			output("Warning: couldn't find any files for the 'wipe' command\n");
		else if (m_bRegex)
		{	// regex search
			std::regex regex;
			try
			{
				regex.assign(m_sRegex, Command_Regex::m_iMode);
			}
			catch (const std::regex_error &e)
			{
				output("Error: regex_error caught: " + std::string(e.what()) + "\n");
				return 1; // error
			}

			std::vector<int> cells;
			size_t ibuffer = 0;
			for (int i = 0; i < result.size(); ++i)
			{
				ibuffer = result[i].rfind('\\');
				if (ibuffer != std::string::npos)
				{
					if (std::regex_match(result[i].substr(++ibuffer), regex))
						cells.push_back(i);
				}
				else if (std::regex_match(result[i], regex))
					cells.push_back(i);
			}

			if (cells.empty())
				output("Warning: couldn't find any files by regex expression for the 'wipe' command\n");
			else
			{
				for (auto &i : cells)
				{
					std::string &it = result[i];
					openToWipe(it);
				}
			}
		}
		else
		{
			output("Error: regular expression must be specified for 'wipe' command\n");
			return 1;
		}
	}

	return 0; // no error
}

void Command_Wipe::openToWipe(std::string &filename)
{
	std::fstream file;
	file.open(filename.c_str(), std::ios::in || std::ios::out);
	if (!file.is_open())
		output("Warning: couldn't open " + filename + " file for the 'wipe' command\n");
	else
	{
		if (m_iLine > 0)
		{
			if (m_bChosenMode)
			{	// descending

			}
		}
	}
}