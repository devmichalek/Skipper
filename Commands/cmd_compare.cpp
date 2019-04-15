#include "cmd_compare.h"
#include "cmd_regex.h"
#include <filesystem>
#include <fstream>

Command_Compare::Command_Compare(std::vector<std::string> options) : Command(options, Handler::CMD_COMPARE)
{
	m_bEmpty = false;
	m_bHelp = false;
	m_bFile= false;
	m_bDirectory = false;
	m_bRegex = false;
	m_sFirstInstance = "";
	m_sSecondInstance = "";
	m_sDirectory = "";
}

bool Command_Compare::parse()
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
				else if (it == "--file") { m_bFile = true; }
				else if (it == "--regex") { m_bRegex = true; }
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
				if (it.find('f') != std::string::npos) { m_bFile = true; }
				if (it.find('r') != std::string::npos) { m_bRegex = true; }
			}
		}
		else
		{
			if (m_bDirectory && m_sDirectory.empty()) { m_sDirectory = it; }
			else if (m_bFile && m_sFirstInstance.empty()) { m_sFirstInstance = it; }
			else if (m_bFile && m_sSecondInstance.empty()) { m_sSecondInstance = it; }
			else if (m_bRegex && m_sFirstInstance.empty()) { m_sFirstInstance = it; }
			else if (m_bRegex && m_sSecondInstance.empty()) { m_sSecondInstance = it; }
			else
			{
				output("Error: too many arguments for 'compare' command\n");
				return false;
			}
		}
	}

	return true; // no error
}

int Command_Compare::run()
{
	if (m_bEmpty)
	{
		output("Error: 'compare' command must at least contain one argument\n");
		return 1;
	}
	else if (m_bHelp)
		output(help());
	else
	{
		if (m_bDirectory && m_sDirectory.empty())
		{
			output("Error: missing <directory name> for --directory switch for 'compare' command\n");
			return 1;
		}

		if (!m_bFile && !m_bRegex)
		{
			output("Error: missing --file switch for 'compare' command\n");
			return 1;
		}

		if (m_bFile && (m_sFirstInstance.empty() || m_sSecondInstance.empty()))
		{
			output("Error: missing <first file name> or <second file name> for --file option for 'compare' command\n");
			return 1;
		}
		else if (m_bRegex && (m_sFirstInstance.empty() || m_sSecondInstance.empty()))
		{
			output("Error: missing <first regex exp> or <second regex exp> for --regex option for 'compare' command\n");
			return 1;
		}

		if (m_bDirectory)
		{
			std::string path = m_sDirectory;
			if (path.back() != '/' && path.back() != '\\')
				path += '/';

			if (m_bFile) {
				m_sFirstInstance = path + m_sFirstInstance;
				m_sSecondInstance = path + m_sSecondInstance;
			}
		}
			
		if (m_bFile)
			compare(m_sFirstInstance, m_sSecondInstance);
		else
		{
			std::vector<std::string> result;
			for (const auto & entry : std::filesystem::directory_iterator(m_sDirectory))
				result.push_back(entry.path().string());

			if (result.empty())
				output("Warning: couldn't find any files\n");
			else if (m_bRegex)
			{	// regex search
				std::regex regex_a;
				try {
					regex_a.assign(m_sFirstInstance, Command_Regex::m_iMode);
				}
				catch (const std::regex_error &e)
				{
					output("Error: regex_error caught: " + std::string(e.what()) + "\n");
					return 1; // error
				}

				std::regex regex_b;
				try {
					regex_b.assign(m_sSecondInstance, Command_Regex::m_iMode);
				}
				catch (const std::regex_error &e)
				{
					output("Error: regex_error caught: " + std::string(e.what()) + "\n");
					return 1; // error
				}

				std::vector<int> cells_a;
				std::vector<int> cells_b;
				size_t ibuffer = 0;
				for (int i = 0; i < result.size(); ++i)
				{
					ibuffer = result[i].rfind('\\');
					if (ibuffer != std::string::npos)
					{
						if (std::regex_match(result[i].substr(++ibuffer), regex_a))
							cells_a.push_back(i);
						if (std::regex_match(result[i].substr(++ibuffer), regex_b))
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
					output("Warning: couldn't find any files by regex expression\n");
				else
				{
					if (cells_a.size() != cells_b.size())
						output("Warning: different vectors, one regex expression matched more files, this can lead to wrong results...\n");

					int count = 0;
					size_t length = cells_a.size() < cells_b.size() ? cells_a.size() : cells_b.size();
					for (size_t j = 0; j < length; ++j)
						compare(result[cells_a[j]], result[cells_b[j]]);
				}
			}
		}
	}

	return 0; // no error
}

void Command_Compare::compare(std::string &a, std::string &b)
{
	std::ifstream fa;
	fa.open(a);
	if (!fa.is_open())
	{
		output("Warning: Cannot open " + a + " file\n");
		return;
	}

	std::ifstream fb;
	fb.open(b);
	if (!fb.is_open())
	{
		output("Warning: Cannot open " + b + " file\n");
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
	{
		fa.close();
		fb.close();
		std::ifstream faa(a, std::ifstream::ate | std::ifstream::binary);
		std::ifstream fbb(b, std::ifstream::ate | std::ifstream::binary);
		if (faa.tellg() != fbb.tellg())
			output("Comparing \"" + a + "\" against \"" + b + "\", found difference in file sizes!\n");
	}
}