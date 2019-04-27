#pragma once
#include "cmd.h"

class Command_Remove final : public Command
{
	bool m_bEmpty;
	bool m_bHelp;
	bool m_bDirectory;
	bool m_bVerbose;
	bool m_bRecursive;
	bool m_bRegex;
	std::string m_sDirectory;
	std::string m_sRegex;

public:
	explicit Command_Remove(std::vector<std::string> options);
	~Command_Remove() {} // not needed, no objects on heap
	bool parse(const char* filename, int &line);
	int run();

	static std::string help()
	{
		std::string a = "\nDefinition:\n";
		std::string b = "\tremove - removes files and directories\n";
		std::string c = "\nSyntax:\n";
		std::string d = "\t[-h --help] - prints help\n";
		std::string e = "\t[-d --directory <directory name>] - searches in requested directory, if not specified searches in current directory\n";
		std::string f = "\t[-v --verbose] - explain at all times what is being done\n";
		std::string g = "\t[-r --recursive] - searches recursively\n";
		std::string h = "\t[<regular expression>] - specifies regular expression pattern\n";
		std::string i = "\n";
		return a + b + c + d + e + f + g + h + i;
	}

	static std::string assist()
	{
		return "  remove\n\t[-h --help]\n\t[-d --directory <directory name>]\n\t[-v --verbose]\n\t[-r --recursive]\n\t[<regular expression>]\n";
	}
};