#include "cmd_list.h"
#include "cmd_regex.h"
#include <filesystem>

Command_List::Command_List(std::vector<std::string> options) : Command(options)
{
	m_bEmpty = false;
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
					output("Error: cannot resolve " + it  + " switch\n");
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
				output("Error: too many arguments for 'list' command\n");
				return false;
			}
		}
	}

	return true; // no error
}

int Command_List::run()
{
	if (m_bHelp)
		output(help());
	else
	{
		std::filesystem::path path;
		if (m_bEmpty)
			path = std::filesystem::current_path();
		else if (m_bDirectory)
			path = m_sDirectory;

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
			output("Warning: couldn't find any files\n");
		else if (m_bRegex)
		{	// regex search
			std::regex regex;
			try
			{
				regex.assign(m_sRegex, Command_Regex::m_iMode);
			}
			catch (const std::regex_error &e)
			{
				output("Error: regex_error caught: " + std::string(e.what()) + "\n");
				return 1; // error
			}

			std::vector<int> cells;
			size_t ibuffer = 0;
			for (int i = 0; i < result.size(); ++i)
			{
				ibuffer = result[i].rfind('\\');
				if (ibuffer != std::string::npos)
				{
					if (std::regex_match(result[i].substr(++ibuffer), regex))
						cells.push_back(i);
				}
				else if (std::regex_match(result[i], regex))
					cells.push_back(i);
			}
				
			if (cells.empty())
				output("Warning: couldn't find any files by regex expression\n");
			else
			{
				int count = 0;
				for (auto &i : cells)
				{
					std::string &it = result[i];
					count = (int)std::count_if(it.begin(), it.end(), [](char &i) { return i == '\\'; });
					std::string buffer(count, ' ');
					output(buffer + it + "\n");
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
				output(buffer + it + "\n");
			}
		}
	}

	return 0; // no error
}