#pragma once
#include <vector>
#include <string>

enum class Subtype
{	// Active subtypes.
	COMMAND = 0, // !command
	OPTION,	// -o --option
	REGEX, // [0-9]+\.txt
	REDIRECTION, // < << > >>
	FILE, // file001.txt
	WORD, // 0 sth
};

struct Arguments
{
	std::vector<Subtype> subtypes;
	std::vector<std::string> switches;

	template<Subtype subtype>
	bool add(std::string _switch, std::string &error_msg)
	{
		if (!subtypes.empty())
		{
			// Check redirection duplicates.
			if (subtypes.back() == Subtype::FILE)
			{
				auto it = std::find(subtypes.begin(), subtypes.end(), Subtype::REDIRECTION);
				if (it != subtypes.end())
				{	// Error.
					error_msg = "<redirection> <file name> is already defined";
					return false;
				}
			}

			// Check if command was already added.
			if (subtype == Subtype::COMMAND)
			{
				auto it = std::find(subtypes.begin(), subtypes.end(), Subtype::COMMAND);
				if (it != subtypes.end())
				{	// Error.
					error_msg = "!<command> is already defined";
					return false;
				}
			}

			// Check redirection.
			if (subtypes.back() == Subtype::REDIRECTION)
			{
				if (subtype != Subtype::FILE && subtype != Subtype::REGEX)
				{	// Error.
					error_msg = "missing <file name> for previously defined redirection \"" + switches.back() + "\"";
					return false;
				}
			}
		}

		// Check if it is command.
		else if (subtype != Subtype::COMMAND)
		{	// Error.
			error_msg = "!<command> is not defined";
			return false;
		}

		// Remove quotes.
		if (_switch.front() == '"')
		{
			_switch.pop_back();
			_switch.erase(0, 1);
		}

		subtypes.push_back(subtype);
		switches.push_back(_switch);
		return true;
	}
	void clear()
	{
		subtypes.clear();
		switches.clear();
	}
};