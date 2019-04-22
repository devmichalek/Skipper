#include "cmd_wipe.h"
#include "cmd_regex.h"
#include "Console.h"
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
	m_iRangeFirst = -1;
	m_iRangeLast = -1;
	m_iLine = -1;
}

bool Command_Wipe::parse(const char* filename, int &line)
{
	if (m_options.empty())
		m_bEmpty = true;

	std::string sline = "", srange = "", sset = "";

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
					std::string res = "Cannot resolve " + it + " switch for the 'wipe' command";
					PrintError(filename, line, res.c_str());
					return false;
				}

				if (it.find('h') != std::string::npos) { m_bHelp = true; break; }
				else if (it.find('d') != std::string::npos) { m_bDirectory = true; }
				else if (it.find('m') != std::string::npos) { m_bMode = true; }
				else if (it.find('s') != std::string::npos) { m_bSet = true; }
				else if (it.find('r') != std::string::npos) { m_bRange = true; }
				else if (it.find('l') != std::string::npos) { m_bLine = true; }
				else if (it.find('k') != std::string::npos) { m_bKeyword = true; }
				else
				{
					PrintError(filename, line, "Invalid switch for the 'wipe' command");
					return false;
				}
			}
		}
		else
		{
			if (m_bDirectory && m_sDirectory.empty())	{ m_sDirectory = it; }
			else if (m_bSet && sset.empty())			{ sset = it; }
			else if (m_bRange /*&& srange.empty()*/)	{ srange += it; }
			else if (m_bLine && sline.empty())			{ sline = it; }
			else if (m_bKeyword && m_sKeyword.empty())	{ m_sKeyword = it; }
			else if (m_sRegex.empty())					{ m_bRegex = true; m_sRegex = it; }
			else
			{
				PrintError(filename, line, "Too many arguments for the 'wipe' command");
				return false;
			}
		}
	}

	if (m_bEmpty)
	{
		PrintError(filename, line, "Too little arguments for the 'wipe' command");
		return 1;
	}

	if (m_bMode && m_bSet) {
		if (!srange.empty() || !m_sKeyword.empty()) {
			PrintError(filename, line, "Too many options turned on for the 'wipe' command");
			return false;
		}
	}

	if (m_bSet)
	{	// Evaluate 0 or 1
		if (sset.size() > 1) {
			PrintError(filename, line, "Invalid <new mode> for --set switch for the 'wipe' command");
			return false;
		}

		if (sset[0] != '0' && sset[0] != '1') {
			PrintError(filename, line, "Invalid <new mode> for --set switch, argument can be either 1 or 0, 'wipe' command");
			return false;
		}

		m_bChosenMode = bool(int(sset[0]) - 48);
	}

	// only one option possible
	if (!sline.empty())
	{
		if (!srange.empty() || !m_sKeyword.empty()) {
			PrintError(filename, line, "Too many switches turned on for the 'wipe' command");
			return false;
		}

		for (auto &it : sline)
			if (!isdigit(it)) {
				PrintError(filename, line, "--line switch for the 'wipe' command should contain only digits");
				return false;
			}

		m_iLine = atoi(sline.c_str());
	}
	else if (!srange.empty())
	{
		if (!sline.empty() || !m_sKeyword.empty()) {
			PrintError(filename, line, "Too many switches turned on for the 'wipe' command");
			return false;
		}

		size_t pos = srange.find(' ');
		if (pos == std::string::npos) {
			PrintError(filename, line, "Argument of --range switch for the 'wipe' command is not correctly specified");
			return false;
		}

		std::string sStart = srange.substr(0, pos);
		for (auto &it : sStart)
			if (!isdigit(it))
			{
				PrintError(filename, line, "<first line> of --range switch for the 'wipe' command should contain only digits");
				return false;
			}
		m_iRangeFirst = atoi(sStart.c_str());

		std::string sEnd = srange.substr(pos + 1);
		for (auto &it : sEnd)
			if (!isdigit(it)) {
				PrintError(filename, line, "<last line> of --range switch for the 'wipe' command should contain only digits");
				return false;
			}
		m_iRangeLast = atoi(sEnd.c_str());

		if (m_iRangeFirst >= m_iRangeLast) {
			PrintError(filename, line, "<first line> of --range switch of the 'wipe' command is >= <last line> which does not make sense");
			return false;
		}
	}
	else if (!m_sKeyword.empty())
	{
		if (!sline.empty() || !srange.empty())
		{
			PrintError(filename, line, "Too many switches turned on for the 'wipe' command");
			return false;
		}
	}

	// Success.
	return true;
}

int Command_Wipe::run()
{
	if (m_bHelp)
		output(help());
	else if (m_bMode)
	{
		output("Available modes are either 1 - descending order or 0 - ascending order\n");
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
			output("Warning: could not find any files for the 'wipe' command\n");
		else if (m_bRegex)
		{	// Regular expression search.
			std::regex regex;
			try
			{
				regex.assign(m_sRegex, Command_Regex::m_iMode);
			}
			catch (const std::regex_error &e)
			{
				output("Error: regex_error caught: " + std::string(e.what()) + ", 'wipe' command\n");
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
				output("Warning: could not find any files by regular expression for the 'wipe' command\n");
			else
			{
				for (auto &i : cells)
				{
					std::string &it = result[i];
					wipe(it);
				}
			}
		}
	}

	// Success.
	return 0;
}

void Command_Wipe::wipe(std::string &filename)
{
	std::fstream file;
	file.open(filename.c_str(), std::ios::in);
	if (!file.is_open())
		output("Warning: could not open " + filename + " file for the 'wipe' command\n");
	else
	{	// Action.
		// Get data.
		int lineCounter = 0;
		int foundKeyword = -1;
		std::vector<std::string> fileData;
		std::string line;
		while (std::getline(file, line)) {
			fileData.push_back(line);
			++lineCounter;
			if (m_bKeyword) {
				if (m_bChosenMode && foundKeyword > -1)
					continue;
				if (fileData.back().find(m_sKeyword) != std::string::npos)
					foundKeyword = lineCounter - 1;
			}
		}

		// --keyword
		if (m_bKeyword)
		{
			if (foundKeyword < 0)
				output("Warning: could not wipe file for the 'wipe' command, could not find <keyword> inside the file\n");
			else
			{	// Keyword is found.
				if (m_bChosenMode)
				{
					file.close();
					file.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
					for (int i = 0; i <= foundKeyword; ++i)
						file << fileData[i];
				}
				else
				{
					file.close();
					file.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
					for (int i = foundKeyword; i < fileData.size(); ++i)
						file << fileData[i];
				}

				file.close();
			}
		}
		else
		{
			if (m_bRange)
			{	//  --range
				// do not care about mode
				if (lineCounter < m_iRangeFirst)
					output("Warning: could not wipe file for the 'wipe' command, <first line> is greater than number of lines of file\n");
				else
				{
					file.close();
					file.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
					for (int i = m_iRangeFirst; i <= m_iRangeLast && i < fileData.size(); ++i)
						file << fileData[i];
					file.close();
				}
			}
			else if (m_bLine)
			{	// --line
				if (m_bChosenMode)
				{
					if (lineCounter < m_iLine)
						output("Warning: could not wipe file for the 'wipe' command, <line> is greater than number of lines of file\n");
					else
					{
						file.close();
						file.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
						for (int i = 0; i <= m_iLine && i < fileData.size(); ++i)
							file << fileData[i];
					}
				}
				else
				{
					file.close();
					file.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
					for (int i = m_iLine; i < fileData.size(); ++i)
						file << fileData[i];
				}

				file.close();
			}
		}
	}

	file.close();
}