#pragma once
#include "cmd.h"
#include <filesystem>

class Command_List final : public Command
{
	bool m_bEmpty;
	bool m_bHelp;
	bool m_bDirectory;
	bool m_bRecursive;
	bool m_bRegex;
	std::string m_sDirectory;
	std::string m_sRegex;

public:
	explicit Command_List(std::vector<std::string> options);
	~Command_List() {} // not needed
	bool parse();
	int run();
};