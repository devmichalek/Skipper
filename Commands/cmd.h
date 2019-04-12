#pragma once
#include <string>
#include <vector>
#include "handlers.h"

class Command
{
	Handler m_handler;
	int m_index;
	void* m_flush;
protected:
	std::vector<std::string> m_options;
public:
	static std::string m_global_buffer;
	Command() = delete;
	Command(const std::vector<std::string> &options, Handler);
	virtual ~Command() = default;
	virtual bool parse() = 0;
	virtual int run(void) = 0;
	void force(void*, int);
	Handler handler() { return m_handler; }
protected:
	void output(std::string msg);
};

bool validate(const std::string &base, std::string search);
std::vector<std::string> extract(const std::string &str, int &&i);
