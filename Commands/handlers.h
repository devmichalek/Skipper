#pragma once
#include <string>

typedef int THandler;
enum class Handler : THandler
{
	CMD_HELP = 0,
	CMD_INCLUDE,
	CMD_LIST,
	CMD_REGEX,
	CMD_REMOVE,
	CMD_END
};

extern const char* HandlerIdentTable[];