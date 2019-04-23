#include "cmd_rename.h"
#include "cmd_regex.h"
#include "Console.h"
#include <filesystem>

Command_Rename::Command_Rename(std::vector<std::string> options) : Command(options, Handler::CMD_RENAME)
{
	m_bEmpty = false;
	m_bHelp = false;
	m_bDirectory = false;
	m_bRecursive = false;
	m_sDirectory = "";
	m_sRegex = "";
	m_sNewName = "";
}

bool Command_Rename::parse(const char* filename, int &line)
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
				else if (it == "--directory") { m_bDirectory = true; }
				else if (it == "--recursive") { m_bRecursive = true; }
			}
			else
			{
				if (!validate(it, "hdr"))
				{
					std::string res = "Cannot resolve " + it + " switch for the 'rename' command";
					PrintError(filename, line, res.c_str());
					return false;
				}

				if (it.find('h') != std::string::npos) { m_bHelp = true; break; }
				if (it.find('d') != std::string::npos) { m_bDirectory = true; }
				if (it.find('r') != std::string::npos) { m_bRecursive = true; }
			}
		}
		else
		{
			if (m_bDirectory && m_sDirectory.empty()) { m_sDirectory = it; }
			else if (m_sRegex.empty()) { m_sRegex = it; }
			else if (m_sNewName.empty()) { m_sNewName = it; }
			else
			{
				PrintError(filename, line, "Too many arguments for the 'rename' command");
				return false;
			}
		}
	}

	if (m_bEmpty) {
		PrintError(filename, line, "Too little arguments for the 'rename' command");
		return false;
	}

	if (!m_bHelp)
	{
		if (m_sRegex.empty()) {
			PrintError(filename, line, "Regular expression is not specified for the 'rename' command");
			return false;
		}

		if (m_sNewName.empty()) {
			PrintError(filename, line, "New name is not specified for the 'rename' command");
			return false;
		}
	}

	return true;
}

int Command_Rename::run()
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
			output("Warning: could not find any files for the 'rename' command\n");
		else
		{	// Regular expression search.
			std::regex regex;
			try
			{
				regex.assign(m_sRegex, Command_Regex::m_iMode);
			}
			catch (const std::regex_error &e)
			{
				output("Error: regex_error caught: " + std::string(e.what()) + ", 'rename' command\n");
				return 1;
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
				output("Warning: could not find any files by regular expression for the 'rename' command\n");
			else
			{
				size_t ibuffer;
				std::vector<std::string> dirs;
				std::vector<int> counters;
				std::string newDirectory;
				std::string newName;
				for (auto &i : cells)
				{	// Make path relative if possible.
					std::string &it = result[i];
					ibuffer = result[i].rfind('\\');
					if (ibuffer == std::string::npos)
						ibuffer = result[i].rfind('/');
					if (ibuffer != std::string::npos)
					{
						std::string newDirectory = it.substr(0, ibuffer);
						auto jt = std::find(dirs.begin(), dirs.end(), newDirectory);
						if (jt == dirs.end())
						{
							counters.push_back(0);
							dirs.push_back(newDirectory);
						}
						else
							++counters[std::distance(dirs.begin(), jt)];
						newName = m_sNewName;
						ibuffer = newName.rfind('.');
						if (ibuffer != std::string::npos)
							newName = newName.substr(0, ibuffer) + std::to_string(counters[std::distance(dirs.begin(), jt)]) + newName.substr(++ibuffer);
						else
							newName += std::to_string(counters[std::distance(dirs.begin(), jt)]);
					}

					// Rename.
					std::filesystem::rename(it, newName);
				}
			}
		}
	}

	// Success.
	return 0;
}