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
	static std::regex::flag_type m_iMode;

public:
	explicit Command_Regex(std::vector<std::string> options);
	Command_Regex() = delete;
	~Command_Regex() {} // not needed, nothing on heap
	bool parse();
	int run();
private:
	std::string getModeString(const int &);
	int getModeByString(const std::string &);
};