#pragma once
#include "cmd.h"
#include "CommonScope.h"
#include "ConcurrentScope.h"
#include "RegularScope.h"

#include <limits>
#include <string>
#include <fstream>
#include <iostream>

class Interpreter final
{
	Command* m_pCmd;
	std::string m_sPathToFile;
public:
	bool m_bExit;
	bool m_bError;
	RegularScope* m_pTree;
public:
	explicit Interpreter();

	/*---- Unique ----*/
	void exit();

	/*---- Scans file, sends data to parse function ----*/
	bool scan(const char*);
private:
	/*---- Connects nodes, creates a tree of scopes ----*/
	bool connect(std::fstream &, CommonScope*&, int&, int&, int&);

	/*---- Parses line to Yacc which calls analyze function ----*/
	void parse(const char*);
public:
	/*---- Analyzes input from Yacc, sets output ----*/
	void analyze(std::string*);
};