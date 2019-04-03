#pragma once
#include <stdio.h>
#include <string>
#include <vector>

class Interpreter
{
public: // public members
	bool bExit;
	bool bError;

public: // public methods
	Interpreter();
	Interpreter(const Interpreter &) = default;

	/*---- Unique Commands ----*/
	void exit();

	/*---- Parse ----*/
	void parse(std::string* msg);
};