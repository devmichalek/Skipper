#include "cmd_remove.h"
#include "cmd_regex.h"
#include "Console.h"
#include <filesystem>

Command_Remove::Command_Remove(std::vector<std::string> options) : Command(options, Handler::CMD_REMOVE)
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

bool Command_Remove::parse(const char* filename, int &line)
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
				if (!validate(it, "hdvr"))
				{
					std::string res = "Cannot resolve " + it + " switch for the 'remove' command";
					PrintError(filename, line, res.c_str());
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
				PrintError(filename, line, "Too many arguments for the 'remove' command\n");
				return false;
			}
		}
	}

	if (m_bEmpty) {
		PrintError(filename, line, "Too little arguments for the 'remove' command");
		return 1;
	}

	if (m_bDirectory && m_sDirectory.empty()) {
		PrintError(filename, line, "Missing <directory name> for --directory switch for the 'remove' command");
		return false;
	}

	return true;
}

int Command_Remove::run()
{
	if (m_bHelp)
		output(help());
	else
	{
		std::filesystem::path path;
		if (m_bDirectory)
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
			output("Warning: could not find any files for the 'remove' command\n");
		else if (m_bRegex)
		{	// Regular expression search.
			std::regex regex;
			try
			{
				regex.assign(m_sRegex, Command_Regex::m_iMode);
			}
			catch (const std::regex_error &e)
			{
				output("Error: regex_error caught: " + std::string(e.what()) + ", 'remove' command\n");
				return 1; // error
			}

			std::vector<int> cells;
			size_t ibuffer = 0;
			for (int i = 0; i < result.size(); ++i)
			{
				std::string &it = result[i];
				ibuffer = result[i].rfind('\\');
				if (ibuffer == std::string::npos)
					ibuffer = result[i].rfind('/');
				if (ibuffer != std::string::npos)
				{
					if (std::regex_match(result[i].substr(++ibuffer), regex))
						cells.push_back(i);
				}
				else if (std::regex_match(result[i], regex))
					cells.push_back(i);
			}

			if (cells.empty())
				output("Warning: could not find any files by regular expression key for the 'remove' command\n");
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
	}

	// Success.
	return 0;
}