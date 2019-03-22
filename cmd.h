#pragma once
#include <string>
#include <vector>

class Command
{
protected:
	std::vector<std::string> m_options;
public:
	Command() = default;
	Command(const Command &) = default;
	Command(const std::vector<std::string> &options);
	virtual ~Command() = default;
	virtual bool parse() = 0;
	virtual int run(void) = 0;
};

bool validate(const std::string &base, std::string search);
std::vector<std::string> extract(const std::string &str, int &&i);