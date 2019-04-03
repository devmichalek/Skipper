#include "cmd.h"

std::string Command::m_global_buffer = "";

Command::Command(const std::vector<std::string> &options)
{
	m_flush = nullptr;
	m_index = -1;
	m_options = options;
}

void Command::force(void* fun)
{
	this->m_flush = fun;
}

void Command::output(std::string msg)
{
	void (*flush)(std::string &&, int &&) = (void(*) (std::string &&, int &&))m_flush;
	flush(std::move(msg), std::move(m_index));
}

bool validate(const std::string &base, std::string search)
{
	unsigned i = 0;
	if (base[0] == '-')
		++i;
	for (; i < base.size(); ++i)
	{
		unsigned j = 0;
		bool found = false;
		for (; j < search.size(); ++j)
		{
			if (base[i] == search[j])
			{
				found = true;
				search.erase(j, 1);
				break;
			}
		}

		if (!found)
			return false;
	}

	return true;
}

std::vector<std::string> extract(const std::string &str, int &&i)
{
	std::vector<std::string> vec;
	std::string buffer = "";
	bool quotes = false;

	for (;; ++i)
	{
		if (i >= str.size())
		{
			if (!buffer.empty())
				vec.push_back(buffer);
			buffer = "";
			break;
		}

		if (str[i] == '"')
		{
			if (quotes)
			{
				if (!buffer.empty())
					vec.push_back(buffer);
				buffer = "";
				++i;
			}
			quotes = !quotes;
		}
		else if (str[i] == ' ' && !quotes)
		{
			if (!buffer.empty())
				vec.push_back(buffer);
			buffer = "";
		}
		else
			buffer += str[i];
	}

	return vec;
}