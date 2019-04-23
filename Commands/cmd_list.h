#pragma once
#include "cmd.h"

class Command_List final : public Command
{
	bool m_bEmpty;
	bool m_bHelp;
	bool m_bDirectory;
	bool m_bRecursive;
	bool m_bRegex;
	std::string m_sDirectory;
	std::string m_sRegex;

public:
	explicit Command_List(std::vector<std::string> options);
	~Command_List() {}
	bool parse(const char* filename, int &line);
	int run();

	static std::string help()
	{
		std::string a = "\nDefinition:\n";
		std::string b = "\tlist - gives list of files and directories\n";
		std::string c = "\nSyntax:\n";
		std::string d = "\t[-h --help] - prints help\n";
		std::string e = "\t[-d --directory <directory name>] - searches in requested directory, if not specified searches in current directory\n";
		std::string f = "\t[-r --recursive] - searches directories recursively\n";
		std::string g = "\t[<regular expression>] - specifies regular expression pattern\n";
		std::string h = "\n";
		return a + b + c + d + e + f + g + h;
	}

	static std::string assist()
	{
		return "  list\n\t[-h --help]\n\t[-d --directory <directory name>]\n\t[-r --recursive]\n\t[<regular expression>]\n";
	}
};