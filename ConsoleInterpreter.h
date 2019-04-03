#pragma once
#include <stdio.h>
#include <string>
#include <vector>

class ConsoleInterpreter
{
public: // public members
	bool bExit;
	bool bError;
	int iHelp;

public: // public methods
	ConsoleInterpreter();
	ConsoleInterpreter(const ConsoleInterpreter &) = default;

	/*---- Unique Commands ----*/
	void exit();
	void help();

	/*---- Parse ----*/
	void parse(std::string* msg);
};