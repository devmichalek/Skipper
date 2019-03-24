#pragma once
#include "cmd.h"
#include <filesystem>

class Command_Remove final : public Command
{
	bool m_bEmpty;
	bool m_bHelp;
	bool m_bDirectory;
	bool m_bVerbose;
	bool m_bRecursive;
	bool m_bRegex;
	std::string m_sDirectory;
	std::string m_sRegex;

public:
	explicit Command_Remove(std::vector<std::string> options);
	~Command_Remove() {} // not needed, no objects on heap
	bool parse();
	int run();
};