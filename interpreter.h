#pragma once
#include <stdio.h>
#include <string>

class Interpreter
{
public: // public members
	bool bExit;
	int iHelp;

public: // public methods
	Interpreter();
	Interpreter(const Interpreter &rhs) = default;

	/*---- Unique Commands ----*/
	void exit();
	void help();

	/*---- Parse ----*/
	void parse(std::string* msg);
};