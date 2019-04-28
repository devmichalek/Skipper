#include "cmd_search.h"
#include "cmd_regex.h"
#include "Console.h"
#include <filesystem>
#include <fstream>

Command_Search::Command_Search(std::vector<std::string> options) : Command(options, Handler::CMD_SEARCH)
{
	m_bEmpty = false;
	m_bHelp = false;
	m_bDirectory = false;
	m_bRecursive = false;
	m_bRegex = false;
	m_bRegexSearch = false;
	m_sDirectory = "";
	m_sRegex = "";
	m_sRegexSearch = "";
}

bool Command_Search::parse(const char* filename, int &line)
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
					std::string res = "Cannot resolve " + it  + " switch for the 'search' command";
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
			if (m_bDirectory && m_sDirectory.empty())	{ m_sDirectory = it; }
			else if (m_sRegex.empty())					{ m_bRegex = true; m_sRegex = it; }
			else if (m_sRegexSearch.empty())			{ m_bRegexSearch = true; m_sRegexSearch = it; }
			else
			{
				PrintError(filename, line, "Too many arguments for the 'search' command\n");
				return false;
			}
		}
	}

	if (!m_bHelp)
	{
		if (m_sRegex.empty())
		{
			PrintError(filename, line, "Missing <file regular expression> for the 'search' command");
			return false;
		}

		if (m_sRegexSearch.empty())
		{
			PrintError(filename, line, "Missing <search regular expression> for the 'search' command");
			return false;
		}
	}

	// Success.
	return true;
}

int Command_Search::run()
{
	if (m_bHelp)
		output(help());
	else
	{
		std::filesystem::path path;
		if (m_bEmpty)
			m_sDirectory = std::filesystem::current_path().string();

		std::vector<std::string> result = entries(m_sDirectory, m_bRecursive);
		if (result.empty())
			output("Warning: could not find any files for the 'search' command\n");
		else if (m_bRegex)
		{	// regex search
			std::regex regex;
			try {
				regex.assign(m_sRegex, Command_Regex::m_iMode);
			}
			catch (const std::regex_error &e) {
				output("Error: regex_error caught: " + std::string(e.what()) + ", 'search' command\n");
				return 1; // Error.
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
				output("Warning: could not find any files by regular expression for the 'search' command\n");
			else
			{
				std::regex sregex;
				try {
					sregex.assign(m_sRegexSearch, Command_Regex::m_iMode);
				}
				catch (const std::regex_error &e) {
					output("Error: regex_error caught: " + std::string(e.what()) + ", 'search' command\n");
					return 1; // Error.
				}

				for (auto &i : cells)
				{
					std::string &it = result[i];
					search(it, sregex);
				}	
			}
		}
	}

	// Success.
	return 0;
}

void Command_Search::search(std::string &filename, std::regex &pattern)
{
	std::fstream file;
	file.open(filename.c_str(), std::ios::in);
	if (!file.is_open())
		output("Warning: could not open " + filename + " file for the 'search' command\n");
	else
	{
		int lineCounter = 0;
		std::vector<std::string> foundPatterns = { std::string("Searching for pattern: \"" + m_sRegexSearch + "\" inside file: \"" + filename + "\"") };
		std::string line;
		while (std::getline(file, line))
		{
			++lineCounter;
			std::sregex_token_iterator jt(line.cbegin(), line.cend(), pattern);
			std::sregex_token_iterator rend;
			if (jt != rend)
			{
				foundPatterns.push_back("Line " + std::to_string(lineCounter) + " ");
				foundPatterns.back() += jt->str(); ++jt;
				for (; jt != rend; ++jt)
					foundPatterns.back() += ", " + jt->str();
			}
		}
		if (foundPatterns.size() == 1)
			foundPatterns.push_back("Could not find any matching patterns");

		for (auto &it : foundPatterns) {
			it += "\n";
			output(it.c_str());
		}
	}

	file.close();
}