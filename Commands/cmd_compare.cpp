#include "cmd_compare.h"
#include "cmd_regex.h"
#include "Console.h"
#include <filesystem>
#include <fstream>

Command_Compare::Command_Compare(std::vector<std::string> options) : Command(options, Handler::CMD_COMPARE)
{
	m_bEmpty = false;
	m_bHelp = false;
	m_bFile= false;
	m_bDirectory = false;
	m_bRecursive = false;
	m_bRegex = false;
	m_sDirectory = "";
	m_sTest = "";
	m_sPattern = "";
}

bool Command_Compare::parse(const char* filename, int &line)
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
					std::string res = "Cannot resolve " + it + " switch for the 'compare' command";
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
					PrintError(filename, line, "Switch --file and --regex cannot be specified together for the 'compare' command");
					return false;
				}
				else if (m_bFile || m_bRegex)
				{
					if (m_sTest.empty())
						m_sTest = it;
					else if (m_sPattern.empty())
						m_sPattern = it;
					else {
						PrintError(filename, line, "Too many arguments for the 'compare' command");
						return false;
					}
				}
				else {
					PrintError(filename, line, "Either switch --file or --regex is not specified for the 'compare' command");
					return false;
				}
			}
		}
	}

	if (m_bEmpty) {
		PrintError(filename, line, "'compare' command must at least contain one argument");
		return false;
	}
	
	if (m_bDirectory && m_sDirectory.empty()) {
		PrintError(filename, line, "Missing <directory name> for --directory switch for the 'compare' command");
		return false;
	}

	if (!m_bHelp)
	{
		if (!m_bFile && !m_bRegex) {
			PrintError(filename, line, "Missing either --file or --regex switch for the 'compare' command");
			return false;
		}

		if (m_bFile) {
			if (m_sTest.empty()) {
				PrintError(filename, line, "Missing <test file name> for --file switch for the 'compare' command");
				return false;
			}
			else if (m_sPattern.empty()) {
				PrintError(filename, line, "Missing <pattern file name> for --file switch for the 'compare' command");
				return false;
			}
		}
		else if (m_bRegex)
		{
			if (m_sTest.empty()) {
				PrintError(filename, line, "Missing <test regular expression> for --regex switch for the 'compare' command");
				return false;
			}
			else if (m_sPattern.empty()) {
				PrintError(filename, line, "Missing <pattern regular expression> for --regex switch for the 'compare' command");
				return false;
			}
		}
	}

	return true;
}

int Command_Compare::run()
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
				m_sTest = m_sDirectory + m_sTest;
				m_sPattern = m_sDirectory + m_sPattern;
			}

			compare(m_sTest, m_sPattern);
		}
		else
		{
			if (!m_bDirectory) // If directory is not set, start from current.
				m_sDirectory = std::filesystem::current_path().string();

			std::vector<std::string> result;
			if (m_bRecursive) {
				for (const auto & entry : std::filesystem::recursive_directory_iterator(m_sDirectory))
					result.push_back(entry.path().string());
			}
			else {
				for (const auto & entry : std::filesystem::directory_iterator(m_sDirectory))
					result.push_back(entry.path().string());
			}

			if (result.empty())
				output("Warning: could not find any files by regular expression for the 'compare' command\n");
			else if (m_bRegex)
			{	// Regular expression search.
				std::regex regex_a;
				try {
					regex_a.assign(m_sTest, Command_Regex::m_iMode);
				}
				catch (const std::regex_error &e) {
					output("Error: regex_error caught: " + std::string(e.what()) + ", 'compare' command\n");
					return 1; // error
				}

				std::regex regex_b;
				try {
					regex_b.assign(m_sPattern, Command_Regex::m_iMode);
				}
				catch (const std::regex_error &e) {
					output("Error: regex_error caught: " + std::string(e.what()) + ", 'compare' command\n");
					return 1; // error
				}

				std::vector<int> cells_a;
				std::vector<int> cells_b;
				size_t ibuffer = 0;
				for (int i = 0; i < result.size(); ++i)
				{
					ibuffer = result[i].rfind('\\');
					if (ibuffer == std::string::npos)
						ibuffer = result[i].rfind('/');
					if (ibuffer != std::string::npos)
					{
						if (std::regex_match(result[i].substr(++ibuffer), regex_a))
							cells_a.push_back(i);
						if (std::regex_match(result[i].substr(ibuffer), regex_b))
							cells_b.push_back(i);
					}
					else
					{
						if (std::regex_match(result[i], regex_a))
							cells_a.push_back(i);
						if (std::regex_match(result[i], regex_b))
							cells_b.push_back(i);
					}
				}

				if (cells_a.empty() || cells_b.empty())
					output("Warning: could not find any files by regular expression for the 'compare' command\n");
				else
				{
					if (cells_a.size() != cells_b.size())
						output("Warning: different vectors, one regular expression matched more files, this can lead to wrong results for the 'compare' command\n");

					int count = 0;
					size_t length = cells_a.size() < cells_b.size() ? cells_a.size() : cells_b.size();
					for (size_t j = 0; j < length; ++j)
						compare(result[cells_a[j]], result[cells_b[j]]);
				}
			}
		}
	}

	return 0; // success
}

void Command_Compare::compare(std::string &a, std::string &b)
{
	std::ifstream fa;
	fa.open(a);
	if (!fa.is_open())
	{
		output("Warning: Cannot open " + a + " file for the 'compare' comamand\n");
		return;
	}

	std::ifstream fb;
	fb.open(b);
	if (!fb.is_open())
	{
		output("Warning: Cannot open " + b + " file for the 'compare' comamand\n");
		return;
	}

	// Files were opened correctly.
	std::string ra, rb;
	int row = -1;
	int line = 1;
	bool difference = false;
	while (std::getline(fa, ra) && std::getline(fb, rb))
	{
		row = -1;
		for (unsigned i = 0; i < ra.size() && i < rb.size(); ++i)
		{
			if (ra[i] != rb[i])
			{
				row = i;
				break;
			}
		}

		if (row == -1 && ra.size() != rb.size())
			row = int(ra.size() < rb.size() ? ra.size() : rb.size());
		
		if (row != -1) {
			output("Comparing \"" + a + "\" against \"" + b + "\", found difference at line " + std::to_string(line) + " row " + std::to_string(row) + "\n");
			difference = true;
		}

		++line;
	}

	if (!difference)
	{	// Check sizes.
		fa.close();
		fb.close();
		std::ifstream faa(a, std::ifstream::ate | std::ifstream::binary);
		std::ifstream fbb(b, std::ifstream::ate | std::ifstream::binary);
		if (faa.tellg() != fbb.tellg())
			output("Comparing \"" + a + "\" against \"" + b + "\", found difference in file sizes!\n");
	}
}