#pragma once
#include "cmd.h"

class Command_Compare final : public Command
{
	bool m_bEmpty;
	bool m_bHelp;
	bool m_bFile;
	bool m_bDirectory;
	bool m_bRecursive;
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
		std::string f = "\t[-r --recursive] - searches recursively\n";
		std::string g = "\t[-f --file <test file name> <pattern file name>] - searches for requested test file and compares it with pattern file\n";
		std::string h = "\t[-R --regex <test regular expression> <pattern regular expression>] - searches for requested files with regular expression, compares 'n' matching test files with 'm' matching pattern files\n";
		std::string i = "\n";
		return a + b + c + d + e + f + g + h + i;
	}

	static std::string assist()
	{
		return "  compare\n\t[-h --help]\n\t[-d --directory <directory name>]\n\t[-r --recursive]\n\t[-f --file <test file name> <pattern file name>]\n\t[-R --regex <test regular expression> <pattern regular expression>]\n";
	}

private:
	void compare(std::string &a, std::string &b);
};