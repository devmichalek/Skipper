#pragma once
#include "cmd.h"

class Command_Compare final : public Command
{
	bool m_bEmpty;
	bool m_bHelp;
	bool m_bFile;
	bool m_bDirectory;
	bool m_bRegex;
	std::string m_sFirstInstance;
	std::string m_sSecondInstance;
	std::string m_sDirectory;

public:
	explicit Command_Compare(std::vector<std::string> options);
	~Command_Compare() {}
	bool parse();
	int run();

	static std::string help()
	{
		std::string a = "\nDefinition:\n";
		std::string b = "\tcompare - compare requested files\n";
		std::string c = "\nSyntax:\n";
		std::string d = "\t[-h --help] - prints help\n";
		std::string e = "\t[-d --directory <directory name>] - searches in requested directory, if not specified searches in current directory\n";
		std::string f = "\t[-f --file <first file name> <second file name>] - searches for requested files (compares 2 files), file name can be either with absolute or relative path\n";
		std::string g = "\t[-r --regex <first regex exp> <second regex exp>] - searches for requested files with regular expression, (compares n matching pattern files, n >= 2)\n";
		std::string h = "\n";
		return a + b + c + d + e + f + g + h;
	}

	static std::string assist()
	{
		return "  compare\n\t[-h --help]\n\t[-d --directory <directory name>]\n\t[-f --file <first file name> <second file name>]\n\t[-r --regex <first regex exp> <second regex exp>]\n";
	}

private:
	void compare(std::string &a, std::string &b);
};