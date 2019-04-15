#include "cmd_include.h"
#include "Console.h"

Command_Include::Command_Include(std::vector<std::string> options) : Command(options, Handler::CMD_INCLUDE)
{
	m_bEmpty = false;
	m_bHelp = false;
	m_bFile= false;
	m_bDirectory = false;
	m_sFile = "";
	m_sDirectory = "";
}

bool Command_Include::parse()
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
			}
			else
			{
				if (!validate(it, "hdf"))
				{
					output("Error: cannot resolve " + it + " switch\n");
					return false;
				}

				if (it.find('h') != std::string::npos) { m_bHelp = true; break; }
				if (it.find('d') != std::string::npos) { m_bDirectory = true; }
				if (it.find('f') != std::string::npos) { m_bFile = true; }
			}
		}
		else
		{
			if (m_bFile && m_sFile.empty()) { m_sFile = it; }
			else if (m_bDirectory && m_sDirectory.empty()) { m_sDirectory = it; }
			else
			{
				output("Error: too many arguments for 'include' command\n");
				return false;
			}
		}
	}

	return true; // no error
}

int Command_Include::run()
{
	m_global_buffer = "";
	if (m_bEmpty)
	{
		output("Error: 'include' command must at least contain one argument\n");
		return 1;
	}
	else if (m_bHelp)
		output(help());
	else
	{
		if (m_bDirectory && m_sDirectory.empty())
		{
			output("Error: missing <directory name> for --directory option for 'include' command\n");
			return 1;
		}

		if (m_bFile && m_sFile.empty())
		{
			output("Error: missing <file name> for --file option for 'include' command\n");
			return 1;
		}
		else if (!m_bFile)
		{
			output("Error: missing --file switch for 'include' command\n");
			return 1;
		}

		if (m_bDirectory)
		{
			m_global_buffer += m_sDirectory;
			if (m_global_buffer.back() != '/' && m_global_buffer.back() != '\\')
				m_global_buffer += '/';
		}
			
		if (m_bFile)
			m_global_buffer += m_sFile;
	}

	return 0; // no error
}
