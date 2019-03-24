#include "cmd_list.h"
#include <regex>

Command_List::Command_List(std::vector<std::string> options) : Command(options)
{
	m_bHelp = false;
	m_bDirectory = false;
	m_bRecursive = false;
	m_bRegex = false;
	m_sDirectory = "";
	m_sRegex = "";
}

bool Command_List::parse()
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
					printf("Error: cannot resolve %s switch\n", it.c_str());
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
			else if (m_sRegex.empty()) { m_bRegex = true; m_sRegex = it; }
			else
			{
				printf("Error: too many arguments for 'list' command\n");
				return false;
			}
		}
	}

	return true; // no error
}

int Command_List::run()
{
	if (m_bHelp)
	{
		printf("\nDefinition:\n");
		printf("\tlist - prints files and directories for requested directory\n");
		printf("\nSyntax:\n");
		printf("\t[-h --help] - prints help\n");
		printf("\t[-d --directory <directory name>] - searches in requested directory, if not specified searches in current directory\n");
		printf("\t[-r --recursive] - searches recursively\n");
		printf("\t[<regular expression>] - searches directory with specified regular expression key\n");
		printf("\n");
	}
	else
	{
		std::filesystem::path path;
		if (m_bEmpty)
			path = std::filesystem::current_path();
		else if (m_bDirectory)
			path = m_sDirectory;
		else
		{ /*error*/ }

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
				for (auto &i : cells)
				{
					std::string &it = result[i];
					count = (int)std::count_if(it.begin(), it.end(), [](char &i) { return i == '\\'; });
					std::string buffer(count, ' ');
					printf("%s%s\n", buffer.c_str(), it.c_str());
				}
			}
		}
		else
		{
			int count = 0;
			for (auto &it : result)
			{
				count = (int)std::count_if(it.begin(), it.end(), [](char &i) { return i == '\\'; });
				std::string buffer(count, ' ');
				printf("%s%s\n", buffer.c_str(), it.c_str());
			}
		}
	}

	return 0; // no error
}