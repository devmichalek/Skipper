#pragma once
#include "cmd.h"

class Command_Run final : public Command
{
	bool m_bEmpty;
	bool m_bHelp;
	bool m_bDirectory;
	bool m_bRecursive;
	bool m_bFile;
	bool m_bRegex;
	std::string m_sDirectory;
	std::string m_sFile;
	std::string m_sRegex;

public:
	explicit Command_Run(std::vector<std::string> options);
	~Command_Run() {}
	bool parse(const char* filename, int &line);
	int run();

	static std::string help()
	{
		std::string a = "\nDefinition:\n";
		std::string b = "\trun - executes external command or any other executable object\n";
		std::string c = "\nSyntax:\n";
		std::string d = "\t[-h --help] - prints help\n";
		std::string e = "\t[-d --directory <directory name>] - searches for object in requested directory, if not specified searches in current directory\n";
		std::string f = "\t[-r --recursive] - searches recursively\n";
		std::string g = "\t[-f --file <object name>] - searches for requested executable object and executes it\n";
		std::string h = "\t[-R --regex <regular expression>] - searches for requested executable objects with regular expression pattern then calls them\n";
		std::string i = "\n";
		return a + b + c + d + e + f + g + h + i;
	}

	static std::string assist()
	{
		return "  run\n\t[-h --help]\n\t[-d --directory <directory name>]\n\t[-r --recursive]\n\t[-e --execute <object name>\n\t[-R --regex <regular expression>]\n";
	}

private:
	bool execute(std::string &link);
};