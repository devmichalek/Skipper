#pragma once
#include "cmd.h"

class Command_Move final : public Command
{
	bool m_bEmpty;
	bool m_bHelp;
	bool m_bDirectory;
	bool m_bRecursive;
	bool m_bIgnore;
	std::string m_sDirectory;
	std::string m_sRegex;
	std::string m_sDestination;

public:
	explicit Command_Move(std::vector<std::string> options);
	~Command_Move() {}
	bool parse(const char* filename, int &line);
	int run();

	static std::string help()
	{
		std::string a = "\nDefinition:\n";
		std::string b = "\tmove - moves selected or matched files into requested directory\n";
		std::string c = "\nSyntax:\n";
		std::string d = "\t[-h --help] - prints help\n";
		std::string e = "\t[-d --directory <directory name>] - searches in requested directory, if not specified searches in current directory\n";
		std::string f = "\t[-r --recursive] - searches recursively\n";
		std::string g = "\t[-i --ignore] - ignores errors and warnings, no break on error\n";
		std::string h = "\t[<regular expression> <destination directory name>] - specifies regular expression key and destination directory for matched objects\n";
		std::string i = "\n";
		return a + b + c + d + e + f + g + h + i;
	}

	static std::string assist()
	{
		return "  move\n\t[-h --help]\n\t[-d --directory <directory name>]\n\t[-r --recursive]\n\t[-i --ignore]\n\t[<regular expression> <destination directory name>]\n";
	}
};