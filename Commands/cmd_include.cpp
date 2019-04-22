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

bool Command_Include::parse(const char* filename, int &line)
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
					std::string res = "Cannot resolve " + it + " switch for the 'include' command";
					PrintError(filename, line, res.c_str());
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
				PrintError(filename, line, "Too many arguments for the 'include' command");
				return false;
			}
		}
	}

	if (m_bEmpty) {
		PrintError(filename, line, "'include' command must at least contain one argument");
		return false;
	}

	if (m_bDirectory && m_sDirectory.empty())
	{
		PrintError(filename, line, "Missing <directory name> for --directory option for the 'include' command");
		return false;
	}

	if (m_bFile && m_sFile.empty())
	{
		PrintError(filename, line, "Missing <file name> for --file option for the 'include' command");
		return false;
	}
	else if (!m_bFile)
	{
		PrintError(filename, line, "Missing --file switch for the 'include' command");
		return false;
	}

	m_global_buffer = "";
	if (!m_bHelp)
	{
		if (m_bDirectory)
		{
			m_global_buffer += m_sDirectory;
			if (m_global_buffer.back() != '/' && m_global_buffer.back() != '\\')
				m_global_buffer += '/';
		}

		if (m_bFile)
			m_global_buffer += m_sFile;
	}

	return true;
}

int Command_Include::run()
{
	if (m_bHelp)
		output(help());

	// Success.
	return 0;
}
