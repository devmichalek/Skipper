#pragma once
#include "cmd.h"

class Command_Wipe final : public Command
{
	bool m_bEmpty;
	bool m_bHelp;
	bool m_bDirectory;
	bool m_bMode;
	bool m_bSet;
	bool m_bRange;
	bool m_bLine;
	bool m_bKeyword;
	bool m_bRegex;
	std::string m_sDirectory;
	std::string m_sKeyword;
	std::string m_sRegex;
	static bool m_bChosenMode; // Top = true, Bot = false;
	int m_iRangeStart;
	int m_iRangeEnd;
	int m_iLine;

public:
	explicit Command_Wipe(std::vector<std::string> options);
	~Command_Wipe() {}
	bool parse();
	int run();

	static std::string help()
	{
		std::string a = "\nDefinition:\n";
		std::string b = "\twipe - wipes requested file in specified range\n";
		std::string c = "\nSyntax:\n";
		std::string d = "\t[-h --help] - prints help\n";
		std::string e = "\t[-d --directory <directory name>] - searches in requested directory, if not specified searches in current directory\n";
		std::string f = "\t[-m --mode] - prints possible modes\n";
		std::string g = "\t[-s --set] - sets new mode\n";
		std::string h = "\t[-r --range <start line> <end line>] - will wipe specified file in requested range\n";
		std::string i = "\t[-l --line <start line>] - will wipe specified file starting from requested line till the end of file\n";
		std::string j = "\t[-k --keyword <keyword>] - will wipe specified file starting from first occurence of keyword till the end of file\n";
		std::string k = "\t[<regular expression>] - searches directory with specified regular expression key\n";
		std::string l = "\n";
		return a + b + c + d + e + f + g + h + i + j + k + l;
	}

	static std::string assist()
	{
		return "  wipe\n\t[-h --help]\n\t[-d --directory <directory name>]\n\t[-m --mode]\n\t[-s --set]\n\t[-r --range <start line> <end line>]\n\t[-l --line <start line>]\n\t[-k --keyword <keyword>]\n\t[<regular expression>]\n";
	}

private:
	void openToWipe(std::string &filename);
};