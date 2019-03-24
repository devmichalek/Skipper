#include "cmd_remove.h"
#include <regex>

Command_Remove::Command_Remove(std::vector<std::string> options) : Command(options)
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

bool Command_Remove::parse()
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
					printf("Error: cannot resolve %s switch\n", it.c_str());
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
				printf("Error: too many arguments for 'remove' command\n");
				return false;
			}
		}
	}

	return true; // no error
}

int Command_Remove::run()
{
	if (m_bHelp)
	{
		printf("\nDefinition:\n");
		printf("\tremove - removes files and directories\n");
		printf("\nSyntax:\n");
		printf("\t[-h --help] - prints help\n");
		printf("\t[-d --directory <directory name>] - searches in requested directory, if not specified searches in current directory\n");
		printf("\t[-v --verbose] - explain at all times what is being done\n");
		printf("\t[-r --recursive] - searches recursively\n");
		printf("\t[<regular expression>] - searches directory with specified regular expression key\n");
		printf("\n");
	}
	else
	{
		std::filesystem::path path;
		if (m_bEmpty)
		{
			printf("Error: too little arguments for 'remove' command\n");
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
			printf("Warning: couldn't find any files\n");
		else if (m_bRegex)
		{	// regex search
			std::regex regex;
			try
			{
				regex.assign(m_sRegex);
			}
			catch (const std::regex_error &e)
			{
				printf("Error: regex_error caught: %s\n", e.what());
				return 1; // error
			}

			std::vector<int> cells;
			size_t ibuffer = 0;
			for (int i = 0; i < result.size(); ++i)
			{
				ibuffer = result[i].rfind('\\');
				if (ibuffer != std::string::npos)
				{
					if (std::regex_match(result[i].substr(ibuffer), regex))
						cells.push_back(i);
				}
				else if (std::regex_match(result[i], regex))
					cells.push_back(i);
			}

			if (cells.empty())
				printf("Warning: couldn't find any files by regex expression\n");
			else
			{
				int count = 0;
				int iRes = 0;
				for (auto &i : cells)
				{
					std::string &it = result[i];
					iRes = std::filesystem::remove(it);
					if (m_bVerbose && iRes > 0)
						printf("Deleted %s\n", it.c_str());
				}
			}
		}
		else
		{
			printf("Error: regular expression is not specified\n");
			return 1;
		}
	}

	return 0; // no error
}