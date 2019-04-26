#pragma once
#include "cmd.h"

class Command_Run final : public Command
{
	bool m_bEmpty;
	bool m_bHelp;
	bool m_bDirectory;
	bool m_bExecute;
	bool m_bRegex;
	std::string m_sDirectory;
	std::string m_sExecute;
	std::string m_sRegex;

public:
	explicit Command_Run(std::vector<std::string> options);
	~Command_Run() {}
	bool parse(const char* filename, int &line);
	int run();

	static std::string help()
	{
		std::string a = "\nDefinition:\n";
		std::string b = "\trun - runs external command or any other executable object\n";
		std::string c = "\nSyntax:\n";
		std::string d = "\t[-h --help] - prints help\n";
		std::string e = "\t[-d --directory <directory name>] - searches for object in requested directory, if not specified searches in current directory\n";
		std::string f = "\t[-e --execute <object name>] - searches for requested executable object and calls it\n";
		std::string g = "\t[-r --regex <regular expression>] - searches for requested executable objects with regular expression pattern then calls them\n";
		std::string h = "\n";
		return a + b + c + d + e + f + g + h;
	}

	static std::string assist()
	{
		return "  run\n\t[-h --help]\n\t[-d --directory <directory name>]\n\t[-e --execute <object name>\n\t[-r --regex <regular expression>]\n";
	}

private:
	bool execute(std::string &link);
};