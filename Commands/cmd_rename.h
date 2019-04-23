#pragma once
#include "cmd.h"

class Command_Rename final : public Command
{
	bool m_bEmpty;
	bool m_bHelp;
	bool m_bDirectory;
	bool m_bRecursive;
	std::string m_sDirectory;
	std::string m_sRegex;
	std::string m_sNewName;

public:
	explicit Command_Rename(std::vector<std::string> options);
	~Command_Rename() {}
	bool parse(const char* filename, int &line);
	int run();

	static std::string help()
	{
		std::string a = "\nDefinition:\n";
		std::string b = "\trename - renames files\n";
		std::string c = "\nSyntax:\n";
		std::string d = "\t[-h --help] - prints help\n";
		std::string e = "\t[-d --directory <directory name>] - searches in requested directory, if not specified searches in current directory\n";
		std::string f = "\t[-r --recursive] - searches recursively\n";
		std::string g = "\t[<regular expression> <new name>] - specifies regular expression key and new name for matched objects\n";
		std::string h = "\n";
		return a + b + c + d + e + f + g + h;
	}

	static std::string assist()
	{
		return "  rename\n\t[-h --help]\n\t[-d --directory <directory name>]\n\t[-r --recursive]\n\t[<regular expression> <new name>]\n";
	}
};