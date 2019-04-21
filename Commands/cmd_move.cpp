#include "cmd_move.h"
#include "cmd_regex.h"
#include <filesystem>

Command_Move::Command_Move(std::vector<std::string> options) : Command(options, Handler::CMD_REMOVE)
{
	m_bEmpty = false;
	m_bHelp = false;
	m_bDirectory = false;
	m_bVerbose = false;
	m_bRecursive = false;
	m_bRegex = false;
	m_sDirectory = "";
	m_sRegex = "";
}

bool Command_Move::parse()
{
	if (m_options.empty())
		m_bEmpty = true;

	for (auto &it : m_options)
	{
		if (it[0] == '-')
		{
			if (it[1] == '-')
			{
				if (it == "--help")				{ m_bHelp = true; break; }
				else if (it == "--directory")	{ m_bDirectory = true; }
				else if (it == "--verbose")		{ m_bVerbose = true; }
				else if (it == "--recursive")	{ m_bRecursive = true; }
			}
			else
			{
				if (!validate(it, "hdfr"))
				{
					output("Error: cannot resolve " + it + " switch\n");
					return false;
				}

				if (it.find('h') != std::string::npos) { m_bHelp = true; break; }
				if (it.find('d') != std::string::npos) { m_bDirectory = true; }
				if (it.find('v') != std::string::npos) { m_bVerbose = true; }
				if (it.find('r') != std::string::npos) { m_bRecursive = true; }
			}
		}
		else
		{
			if (m_bDirectory && m_sDirectory.empty()) { m_sDirectory = it; }
			else if (m_sRegex.empty()) { m_bRegex = true; m_sRegex = it; }
			else
			{
				output("Error: too many arguments for 'remove' command\n");
				return false;
			}
		}
	}

	return true; // no error
}

int Command_Move::run()
{
	if (m_bHelp)
		output(help());
	else
	{
		std::filesystem::path path;
		if (m_bEmpty)
		{
			output("Error: too little arguments for 'remove' command\n");
			return 1;
		}
		else if (m_bDirectory)
			path = m_sDirectory;
		else
			path = std::filesystem::current_path();

		std::vector<std::string> result;
		if (m_bRecursive)
		{
			for (const auto & entry : std::filesystem::recursive_directory_iterator(path))
				result.push_back(entry.path().string());
		}
		else
		{
			for (const auto & entry : std::filesystem::directory_iterator(path))
				result.push_back(entry.path().string());
		}

		if (result.empty())
			output("Warning: couldn't find any files\n");
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
				output("Warning: couldn't find any files by regex expression\n");
			else
			{
				int count = 0;
				int iRes = 0;
				for (auto &i : cells)
				{
					std::string &it = result[i];
					iRes = std::filesystem::remove(it);
					if (m_bVerbose && iRes > 0)
						output("Deleted " + it + "\n");
				}
			}
		}
		else
		{
			output("Error: regular expression is not specified\n");
			return 1;
		}
	}

	return 0; // no error
}