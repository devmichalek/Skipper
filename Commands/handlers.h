#pragma once

typedef int THandler;
enum class Handler : THandler
{
	CMD_COMPARE = 0,
	CMD_COPY,
	CMD_HELP,
	CMD_INCLUDE,
	CMD_LIST,
	CMD_MOVE,
	CMD_REGEX,
	CMD_REMOVE,
	CMD_RENAME,
	CMD_RUN,
	CMD_SEARCH,
	CMD_WAIT,
	CMD_WIPE
};

extern const char* HandlerIdentTable[];