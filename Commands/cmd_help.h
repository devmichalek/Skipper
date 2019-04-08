#pragma once
#include "cmd.h"

class Command_Help final : public Command
{
	bool m_bEmpty;
	bool m_bHelp;

public:
	explicit Command_Help(std::vector<std::string> options);
	~Command_Help() {} // not needed
	bool parse();
	int run();

	static std::string help()
	{
		std::string a = "\nDefinition:\n";
		std::string b = "\thelp - prints short help of all embedded commands\n";
		std::string c = "\nSyntax:\n";
		std::string d = "\t[-h --help] - prints help\n";
		std::string e = "\n";
		return a + b + c + d + e;
	}

	static std::string assist()
	{
		return "  help\n\t[-h --help]\n";
	}
};