#pragma once
#include "cmd.h"
#include <chrono>

class Command_Wait final : public Command
{
	bool m_bEmpty;
	bool m_bHelp;
	bool m_bDuration;
	int m_iDuration;
	int m_iType;

public:
	explicit Command_Wait(std::vector<std::string> options);
	~Command_Wait() {}
	bool parse(const char* filename, int &line);
	int run();

	static std::string help()
	{
		std::string a = "\nDefinition:\n";
		std::string b = "\twait - wait for requested time\n";
		std::string c = "\nSyntax:\n";
		std::string d = "\t[-h --help] - prints help\n";
		std::string e = "\t[-d --duration] - prints available duration times\n";
		std::string f = "\t[<time duration> <type of duration>] - the time to delay the current thread\n";
		std::string g = "\n";
		return a + b + c + d + e + f + g;
	}

	static std::string assist()
	{
		return "  wait\n\t[-h --help]\n\t[-d --duration]\n\t[<time duration> <type of duration>]\n";
	}
};