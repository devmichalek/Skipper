#pragma once
#include "cmd.h"

class Command_Copy final : public Command
{
	bool m_bEmpty;
	bool m_bHelp;
	bool m_bDirectory;
	bool m_bRecursive;
	bool m_bOverwrite;
	bool m_bUpdate;
	bool m_bDirOnly;
	bool m_bIgnore;
	std::string m_sDirectory;
	std::string m_sRegex;
	std::string m_sDestination;

public:
	explicit Command_Copy(std::vector<std::string> options);
	~Command_Copy() {}
	bool parse(const char* filename, int &line);
	int run();

	static std::string help()
	{
		std::string a = "\nDefinition:\n";
		std::string b = "\tcopy - copies requested files into requested directory\n";
		std::string c = "\nSyntax:\n";
		std::string d = "\t[-h --help] - prints help\n";
		std::string e = "\t[-d --directory <directory name>] - searches in requested directory, if not specified searches in current directory\n";
		std::string f = "\t[-r --recursive] - searches recursively, keeps recursive structure\n";
		std::string g = "\t[-o --overwrite] - replaces the existing object\n";
		std::string h = "\t[-u --update] - replaces the existing file only if it is older than the file being copied\n";
		std::string i = "\t[-c --directoryonly] - copies the directory structure, but do not copy any non-directory files\n";
		std::string j = "\t[-i --ignore] - ignores errors and warnings, no break on error\n";
		std::string k = "\t[<regular expression> <destination directory name>] - specifies regular expression key and destination directory for matched objects\n";
		std::string l = "\n";
		return a + b + c + d + e + f + g + h + i+ j + k + l;
	}

	static std::string assist()
	{
		return "  copy\n\t[-h --help]\n\t[-d --directory <directory name>]\n\t[-r --recursive]\n\t[-o --overwrite]\n\t[-u --update]\n\t[-c --directoryonly]\n\t[-i --ignore]\n\t[<regular expression> <destination directory name>]\n";
	}
};