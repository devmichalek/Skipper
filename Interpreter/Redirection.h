#pragma once
#include <string>

enum class RedirectionType
{
	LEFT = 0,
	LEFT_ADD,
	RIGHT,
	RIGHT_ADD
};
typedef std::pair<std::string, RedirectionType> Redirection;