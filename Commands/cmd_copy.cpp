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
	m_bOverwrite = false;
	m_bUpdate = false;
	m_bDirOnly = false;
	m_bIgnore = false;
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
				if (it == "--help")					{ m_bHelp = true; break; }
				else if (it == "--directory")		{ m_bDirectory = true; }
				else if (it == "--recursive")		{ m_bRecursive = true; }
				else if (it == "--overwrite")		{ m_bOverwrite = true; }
				else if (it == "--update")			{ m_bUpdate = true; }
				else if (it == "--directoryonly")	{ m_bDirOnly = true; }
				else if (it == "--ignore")			{ m_bIgnore = true; }
			}
			else
			{
				if (!validate(it, "hdrouci"))
				{
					std::string res = "Cannot resolve " + it + " switch for the 'copy' command";
					PrintError(filename, line, res.c_str());
					return false;
				}

				if (it.find('h') != std::string::npos) { m_bHelp = true; break; }
				if (it.find('d') != std::string::npos) { m_bDirectory = true; }
				if (it.find('r') != std::string::npos) { m_bRecursive = true; }
				if (it.find('o') != std::string::npos) { m_bOverwrite = true; }
				if (it.find('u') != std::string::npos) { m_bUpdate = true; }
				if (it.find('c') != std::string::npos) { m_bDirOnly = true; }
				if (it.find('i') != std::string::npos) { m_bIgnore = true; }
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

	if (m_bOverwrite && m_bUpdate) {
		PrintError(filename, line, "Switches --overwrite and --update cannot be specified together for the 'copy' command");
		return false;
	}

	if (!m_bHelp)
	{
		if (m_sRegex.empty()) {
			PrintError(filename, line, "Regular expression is not specified for the 'copy' command");
			return false;
		}

		if (m_sDestination.empty()) {
			PrintError(filename, line, "Destination directory is not specified for the 'copy' command");
			return false;
		}
	}

	return true;
}

int Command_Copy::run()
{
	if (m_bHelp)
		output(help());
	else
	{
		if (!m_bDirectory)
			m_sDirectory = std::filesystem::current_path().string();

		std::vector<std::string> result = entries(m_sDirectory, m_bRecursive);
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
				std::filesystem::copy_options option = std::filesystem::copy_options::none;
				if (m_bRecursive)
					option |= std::filesystem::copy_options::recursive;
				if (m_bOverwrite)
					option |= std::filesystem::copy_options::overwrite_existing;
				if (m_bUpdate)
					option |= std::filesystem::copy_options::update_existing;
				if (m_bDirOnly)
					option |= std::filesystem::copy_options::directories_only;

				if (m_sDestination.back() != '/' && m_sDestination.back() != '\\')
					m_sDestination += '/';

				size_t ibuffer;
				std::string temp;
				for (auto &i : cells)
				{	// Make path relative if possible.
					std::string &it = result[i];
					ibuffer = result[i].rfind('\\');
					if (ibuffer == std::string::npos)
						ibuffer = result[i].rfind('/');
					if (ibuffer != std::string::npos)
						temp = it.substr(++ibuffer);
					else
						temp = it;

					// Copy.
					try {
						std::filesystem::copy(it, m_sDestination + temp, option);
					}
					catch (std::filesystem::filesystem_error &e) {
						output("Error: filesystem_error caught: " + std::string(e.what()) + ", 'copy' command\n");
						if (!m_bIgnore)
							return 1;
					}
				}
			}
		}
	}

	// Success.
	return 0;
}