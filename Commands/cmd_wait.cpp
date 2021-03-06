#include "cmd_wait.h"
#include "Console.h"
#include <chrono>
#include <thread>

Command_Wait::Command_Wait(std::vector<std::string> options) : Command(options, Handler::CMD_WAIT)
{
	m_bEmpty = false;
	m_bHelp = false;
	m_bDuration = false;
	m_iDuration = 0;
	m_iType = -1;
}

bool Command_Wait::parse(const char* filename, int &line)
{
	if (m_options.empty())
		m_bEmpty = true;

	std::string m_sDuration = "";
	for (auto &it : m_options)
	{
		if (it[0] == '-')
		{
			if (it[1] == '-')
			{
				if (it == "--help")				{ m_bHelp = true; break; }
				else if (it == "--duration")	{ m_bDuration = true; }
			}
			else
			{
				if (!validate(it, "hd"))
				{
					std::string res = "Cannot resolve " + it + " switch for the 'wait' command";
					PrintError(filename, line, res.c_str());
					return false;
				}

				if (it.find('h') != std::string::npos) { m_bHelp = true; break; }
				if (it.find('d') != std::string::npos) { m_bDuration = true; }
			}
		}
		else
		{
			if (m_sDuration.empty()) { m_sDuration = it; }
			else
			{
				PrintError(filename, line, "Too many arguments for the 'wait' command");
				return false;
			}
		}
	}

	if (m_bEmpty) {
		PrintError(filename, line, "Too little arguments for the 'wait' command\n");
		return 1;
	}

	if (!m_bHelp && !m_bDuration && m_sDuration.empty()) {
		PrintError(filename, line, "Missing argument for the 'wait' command");
		return false;
	}

	else if (!m_sDuration.empty())
	{
		std::string sDigit = "";
		for (auto &it : m_sDuration)
		{
			if (isdigit(it))
				sDigit += it;
			else
				break;
		}

		if (sDigit.empty()) {
			PrintError(filename, line, "Missing digit part of argument for the 'wait' command");
			return false;
		}

		m_iDuration = atoi(sDigit.c_str());

		std::string sType = "";
		for (int i = (int)m_sDuration.size() - 1; i >= 0; --i)
		{
			if (isdigit(m_sDuration[i]))
				break;
			else
				sType = m_sDuration[i] + sType;
		}

		if (sType.empty()) {
			PrintError(filename, line, "Missing duration type in argument for the 'wait' command");
			return false;
		}

		if (sType == "ns")			m_iType = 0;
		else if (sType == "us")		m_iType = 1;
		else if (sType == "ms")		m_iType = 2;
		else if (sType == "s")		m_iType = 3;
		else if (sType == "min")	m_iType = 4;
		else if (sType == "h")		m_iType = 5;

		if (m_iType < 0) {
			PrintError(filename, line, "Duration type argument is wrong for the 'wait' command");
			return false;
		}
	}

	return true;
}

int Command_Wait::run()
{
	if (m_bHelp)
		output(help());
	else if (m_bDuration)
	{
		output("Available duration types are:\n");
		output("\tnanoseconds 'ns'\n");
		output("\tmicroseconds 'us'\n");
		output("\tmiliseconds 'ms'\n");
		output("\tseconds 's'\n");
		output("\tminutes 'min'\n");
		output("\thours 'h'\n");
		output("Example:\n");
		output("\t!wait 1000ms\n");
	}
	else
	{
		switch (m_iType)
		{
			case 0: std::this_thread::sleep_for(std::chrono::nanoseconds(m_iDuration));		break;
			case 1: std::this_thread::sleep_for(std::chrono::microseconds(m_iDuration));	break;
			case 2: std::this_thread::sleep_for(std::chrono::milliseconds(m_iDuration));	break;
			case 3: std::this_thread::sleep_for(std::chrono::seconds(m_iDuration));			break;
			case 4: std::this_thread::sleep_for(std::chrono::minutes(m_iDuration));			break;
			case 5: std::this_thread::sleep_for(std::chrono::hours(m_iDuration));			break;
		}
	}

	return 0;
}