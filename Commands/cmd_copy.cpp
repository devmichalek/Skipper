#include "cmd_copy.h"
#include "cmd_regex.h"
#include "Console.h"
#include <filesystem>

Command_Copy::Command_Copy(std::vector<std::string> options) : Command(options, Handler::CMD_COPY)
{
	m_bEmpty = false;
	m_bHelp = false;
	m_bDirectory = false;
	m_bRecursive = false;
	m_sDirectory = "";
	m_sRegex = "";
	m_sDestination = "";
}

bool Command_Copy::parse(const char* filename, int &line)
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
				else if (it == "--recursive")	{ m_bRecursive = true; }
			}
			else
			{
				if (!validate(it, "hdr"))
				{
					std::string res = "Cannot resolve " + it + " switch for the 'copy' command";
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
			else if (m_sDestination.empty()) { m_sDestination = it; }
			else
			{
				PrintError(filename, line, "Too many arguments for the 'copy' command");
				return false;
			}
		}
	}

	if (m_bEmpty) {
		PrintError(filename, line, "Too little arguments for the 'copy' command");
		return false;
	}

	if (m_sRegex.empty())
	{
		PrintError(filename, line, "Regular expression is not specified for the 'copy' command");
		return false;
	}

	if (m_sDestination.empty())
	{
		PrintError(filename, line, "Destination directory is not specified for the 'copy' command");
		return false;
	}

	return true;
}

int Command_Copy::run()
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
			output("Warning: could not find any files for the 'copy' command\n");
		else
		{	// Regular expression search.
			std::regex regex;
			try
			{
				regex.assign(m_sRegex, Command_Regex::m_iMode);
			}
			catch (const std::regex_error &e)
			{
				output("Error: regex_error caught: " + std::string(e.what()) + ", 'copy' command\n");
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
				output("Warning: could not find any files by regular expression for the 'copy' command\n");
			else
			{
				std::filesystem::copy_options option = m_bRecursive ? std::filesystem::copy_options::recursive : std::filesystem::copy_options::none;
				if (m_sDestination.back() != '/' && m_sDestination.back() != '\\')
					m_sDestination += '/';

				size_t ibuffer;
				for (auto &i : cells)
				{	// Make path relative if possible.
					std::string &it = result[i];
					ibuffer = result[i].rfind('\\');
					if (ibuffer == std::string::npos)
						ibuffer = result[i].rfind('/');
					if (ibuffer != std::string::npos)
						it = it.substr(++ibuffer);
					// Copy.
					std::filesystem::copy(it, m_sDestination + it, option);
				}
			}
		}
	}

	// Success.
	return 0;
}