#pragma once

typedef int THandler;
enum class Handler : THandler
{
	CMD_COMPARE = 0,
	CMD_HELP,
	CMD_INCLUDE,
	CMD_LIST,
	CMD_REGEX,
	CMD_REMOVE,
	CMD_WAIT
};

extern const char* HandlerIdentTable[];