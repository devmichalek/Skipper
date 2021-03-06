#include "cmd_run.h"
#include "cmd_regex.h"
#include "Console.h"
#include <filesystem>
#include <array>

Command_Run::Command_Run(std::vector<std::string> options) : Command(options, Handler::CMD_RUN)
{
	m_bEmpty = false;
	m_bHelp = false;
	m_bFile = false;
	m_bDirectory = false;
	m_bRecursive = false;
	m_bRegex = false;
	m_sDirectory = "";
	m_sFile = "";
	m_sRegex = "";
}

bool Command_Run::parse(const char* filename, int &line)
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
				else if (it == "--file")		{ m_bFile = true; }
				else if (it == "--regex")		{ m_bRegex = true; }
			}
			else
			{
				if (!validate(it, "hdrfR"))
				{
					std::string res = "Cannot resolve " + it + " switch for the 'run' command";
					PrintError(filename, line, res.c_str());
					return false;
				}

				if (it.find('h') != std::string::npos) { m_bHelp = true; break; }
				if (it.find('d') != std::string::npos) { m_bDirectory = true; }
				if (it.find('r') != std::string::npos) { m_bRecursive = true; }
				if (it.find('f') != std::string::npos) { m_bFile = true; }
				if (it.find('R') != std::string::npos) { m_bRegex = true; }
			}
		}
		else
		{
			if (m_bDirectory && m_sDirectory.empty())
				m_sDirectory = it;
			else
			{
				if (m_bFile && m_bRegex)
				{
					PrintError(filename, line, "Switches --file and --regex cannot be specified together for the 'run' command");
					return false;
				}
				else if (m_bFile && m_sFile.empty())
					m_sFile = it;
				else if (m_bRegex && m_sRegex.empty())
					m_sRegex = it;
				else {
					PrintError(filename, line, "Either switch --file or --regex is not specified for the 'run' command");
					return false;
				}
			}
		}
	}

	if (m_bEmpty) {
		PrintError(filename, line, "'run' command must at least contain one argument");
		return false;
	}
	
	if (m_bDirectory && m_sDirectory.empty()) {
		PrintError(filename, line, "Missing <directory name> for --directory switch for the 'run' command");
		return false;
	}

	if (!m_bHelp)
	{
		if (!m_bFile && !m_bRegex) {
			PrintError(filename, line, "Missing either --file or --regex switch for the 'run' command");
			return false;
		}

		if (m_bFile) {
			if (m_sFile.empty()) {
				PrintError(filename, line, "Missing <object name> for --file switch for the 'run' command");
				return false;
			}
		}
		else if (m_bRegex) {
			if (m_sRegex.empty()) {
				PrintError(filename, line, "Missing <regular expression> for --regex switch for the 'run' command");
				return false;
			}
		}
	}

	return true;
}

int Command_Run::run()
{
	if (m_bHelp)
		output(help());
	else
	{
		if (m_bFile)
		{
			if (m_bDirectory)
			{
				if (m_sDirectory.back() != '/' && m_sDirectory.back() != '\\')
					m_sDirectory += '/';
				m_sFile = m_sDirectory + m_sFile;
			}

			execute(m_sFile);
		}
		else
		{
			if (!m_bDirectory) // If directory is not set, start from current.
				m_sDirectory = std::filesystem::current_path().string();

			std::vector<std::string> result = entries(m_sDirectory, m_bRecursive);
			if (result.empty())
				output("Warning: could not find any files by regular expression for the 'run' command\n");
			else if (m_bRegex)
			{	// Regular expression search.
				std::regex regex;
				try {
					regex.assign(m_sRegex, Command_Regex::m_iMode);
				}
				catch (const std::regex_error &e) {
					output("Error: regex_error caught: " + std::string(e.what()) + ", 'run' command\n");
					return 1; // error
				}

				std::vector<int> cells;
				size_t ibuffer = 0;
				for (int i = 0; i < result.size(); ++i)
				{
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
					output("Warning: could not find any files by regular expression for the 'run' command\n");
				else
				{
					int count = 0;
					for (size_t j = 0; j < cells.size(); ++j)
						execute(result[cells[j]]);
				}
			}
		}
	}

	return 0; // success
}

bool Command_Run::execute(std::string &link)
{
	std::replace(link.begin(), link.end(), '/', '\\');

	int res = 0;
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(link.c_str(), "r"), _pclose);
	if (!pipe) {
		//throw std::runtime_error("popen() failed");
		res = 1;
	}
	else
	{
		while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
			result += buffer.data();

		output(result);
	}

	return !res;
}