#pragma once
#include "cmd.h"
#include <regex>

class Command_Search final : public Command
{
	bool m_bEmpty;
	bool m_bHelp;
	bool m_bDirectory;
	bool m_bRecursive;
	bool m_bRegex;
	bool m_bRegexSearch;
	std::string m_sDirectory;
	std::string m_sRegex;
	std::string m_sRegexSearch;

public:
	explicit Command_Search(std::vector<std::string> options);
	~Command_Search() {}
	bool parse(const char* filename, int &line);
	int run();

	static std::string help()
	{
		std::string a = "\nDefinition:\n";
		std::string b = "\tsearch - searches through file\n";
		std::string c = "\nSyntax:\n";
		std::string d = "\t[-h --help] - prints help\n";
		std::string e = "\t[-d --directory <directory name>] - searches in requested directory, if not specified searches in current directory\n";
		std::string f = "\t[-r --recursive] - searches directories recursively\n";
		std::string g = "\t[<file regular expression> <search regular expression>] - specifies regular expression pattern for file and search\n";
		std::string h = "\n";
		return a + b + c + d + e + f + g + h;
	}

	static std::string assist()
	{
		return "  search\n\t[-h --help]\n\t[-d --directory <directory name>]\n\t[-r --recursive]\n\t[<file regular expression> <search regular expression>]\n";
	}

private:
	void search(std::string &filename, std::regex &pattern);
};