#pragma once
#include "cmd.h"

class Command_Compare final : public Command
{
	bool m_bEmpty;
	bool m_bHelp;
	bool m_bFile;
	bool m_bDirectory;
	bool m_bRegex;
	std::string m_sDirectory;
	std::string m_sTest;
	std::string m_sPattern;

public:
	explicit Command_Compare(std::vector<std::string> options);
	~Command_Compare() {}
	bool parse(const char* filename, int &line);
	int run();

	static std::string help()
	{
		std::string a = "\nDefinition:\n";
		std::string b = "\tcompare - compares requested file with requested pattern\n";
		std::string c = "\nSyntax:\n";
		std::string d = "\t[-h --help] - prints help\n";
		std::string e = "\t[-d --directory <directory name>] - searches for files in requested directory, if not specified searches in current directory\n";
		std::string f = "\t[-f --file <test file name> <pattern file name>] - searches for requested test file and compares it with pattern file\n";
		std::string g = "\t[-r --regex <test regular expression> <pattern regular expression>] - searches for requested files with regular expression, compares 'n' matching test files with 'm' matching pattern files\n";
		std::string h = "\n";
		return a + b + c + d + e + f + g + h;
	}

	static std::string assist()
	{
		return "  compare\n\t[-h --help]\n\t[-d --directory <directory name>]\n\t[-f --file <test file name> <pattern file name>]\n\t[-r --regex <first regular expression> <second regular expression>]\n";
	}

private:
	void compare(std::string &a, std::string &b);
};