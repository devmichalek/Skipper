#pragma once
#include "cmd.h"

class Command_Echo final : public Command
{
	bool m_bEmpty;
	bool m_bHelp;
	std::string m_sText;

public:
	explicit Command_Echo(std::vector<std::string> options);
	~Command_Echo() {}
	bool parse(const char* filename, int &line);
	int run();

	static std::string help()
	{
		std::string a = "\nDefinition:\n";
		std::string b = "\techo - prints the specified text\n";
		std::string c = "\nSyntax:\n";
		std::string d = "\t[-h --help] - prints help\n";
		std::string e = "\t[<text>] - specifies text to echo\n";
		std::string f = "\n";
		return a + b + c + d + e + f;
	}

	static std::string assist()
	{
		return "  echo\n\t[-h --help]\n\t[<text>]\n";
	}
};