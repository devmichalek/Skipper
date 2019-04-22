#pragma once
#include "cmd.h"
#include <regex>

class Command_Regex final : public Command
{
	bool m_bEmpty;
	bool m_bHelp;
	bool m_bMode;
	bool m_bList;
	bool m_bSetMode;
	std::string m_sSetMode;

public:
	static std::regex::flag_type m_iMode;

	explicit Command_Regex(std::vector<std::string> options);
	Command_Regex() = delete;
	~Command_Regex() {}
	bool parse(const char* filename, int &line);
	int run();

	static std::string help()
	{
		std::string a = "\nDefinition:\n";
		std::string b = "\tregex - gives regular expression information, changes regular expression behaviour\n";
		std::string c = "\nSyntax:\n";
		std::string d = "\t[-h --help] - prints help\n";
		std::string e = "\t[-m --mode] - prints current regular mode\n";
		std::string f = "\t[-l --list] - prints available regular mode\n";
		std::string g = "\t[-s --set <new mode>] - sets new regular mode\n";
		std::string h = "\n";
		return a + b + c + d + e + f + g + h;
	}

	static std::string assist()
	{
		return "  regex\n\t[-h --help]\n\t[-m --mode]\n\t[-l --list]\n\t[-s --set <new mode>]\n";
	}

private:
	std::string getModeString(const int &);
	int getModeByString(const std::string &);
};