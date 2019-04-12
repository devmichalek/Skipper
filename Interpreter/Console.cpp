#include "Console.h"
#include <Windows.h>

void PrintMessage(int color, const char* prefix, const char* &filename, int &line, const char* &msg)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, color);
	printf("%s: ", prefix);
	SetConsoleTextAttribute(hConsole, 7);
	if (filename && line > -1)
		printf("File: \"%s\" Line: %d => %s\n", filename, line, msg);
	else
		printf("%s\n", msg);
}

void PrintError(const char* filename, int line, const char* msg)
{
	PrintMessage(12, "Error", filename, line, msg);
}

void PrintFatalError(const char* filename, int line, const char* msg)
{
	PrintMessage(13, "Fatal Error", filename, line, msg);
}

void PrintWarning(const char* filename, int line, const char* msg)
{
	PrintMessage(14, "Warning", filename, line, msg);
}

void PrintSuccess(const char* filename, int line, const char* msg)
{
	PrintMessage(10, "Success", filename, line, msg);
}