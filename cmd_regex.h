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
	~Command_Regex() {} // not needed, nothing on heap
	bool parse();
	int run();

	static std::string help()
	{
		std::string a = "\nDefinition:\n";
		std::string b = "\tregex - gives regex information, changes regex behaviour\n";
		std::string c = "\nSyntax:\n";
		std::string d = "\t[-h --help] - prints help\n";
		std::string e = "\t[-m --mode] - prints current regex mode\n";
		std::string f = "\t[-l --list] - prints available regex mode\n";
		std::string g = "\t[-s --set <new mode>] - sets new regex mode\n";
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