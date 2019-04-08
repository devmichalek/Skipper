#pragma once
#include <string>
#include <vector>

class Command
{
	void* m_flush;
	int m_index;
protected:
	std::vector<std::string> m_options;
public:
	static std::string m_global_buffer;
	Command() = default;
	Command(const Command &) = default;
	Command(const std::vector<std::string> &options);
	virtual ~Command() = default;
	virtual bool parse() = 0;
	virtual int run(void) = 0;
	void force(void*);
protected:
	void output(std::string msg);
};

bool validate(const std::string &base, std::string search);
std::vector<std::string> extract(const std::string &str, int &&i);
