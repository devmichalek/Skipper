#pragma once
#include "cmd.h"

class Command_Include final : public Command
{	// This command is evaluated during interpretation!
	bool m_bEmpty;
	bool m_bHelp;
	bool m_bFile;
	bool m_bDirectory;
	std::string m_sFile;
	std::string m_sDirectory;

public:
	explicit Command_Include(std::vector<std::string> options);
	~Command_Include() {}
	bool parse(const char* filename, int &line);
	int run();

	static std::string help()
	{
		std::string a = "\nDefinition:\n";
		std::string b = "\tinclude - includes file, replaces its occurence with requested file body during interpretation\n";
		std::string c = "\nSyntax:\n";
		std::string d = "\t[-h --help] - prints help\n";
		std::string e = "\t[-d --directory <directory name>] - searches in requested directory, if not specified searches in current directory\n";
		std::string f = "\t[-f --file <file name>] - searches for requested file, file name can be either with absolute or relative path\n";
		std::string g = "\n";
		return a + b + c + d + e + f + g;
	}

	static std::string assist()
	{
		return "  include\n\t[-h --help]\n\t[-d --directory <directory name>]\n\t[-f --file <file name>]\n";
	}
};