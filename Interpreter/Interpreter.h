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
	Interpreter* m_child;
	std::string m_sPathToFile;
public:
	bool m_bError;
	std::string m_sCatchedMsg;
	std::string m_sFileName;
	RegularScope* m_pTree;
public:
	explicit Interpreter(RegularScope*&);
	~Interpreter();

	/*---- Scans file, sends data to parse function ----*/
	bool scan(const char*, const char*, int);
private:
	/*---- Connects nodes, creates a tree of scopes ----*/
	bool connect(std::fstream &, CommonScope*&, int&, int&, int&);

	/*---- Parses line to Yacc which calls analyze function ----*/
	void parse(const char*);
public:
	/*---- Analyzes input from Yacc, sets output ----*/
	void analyze(std::string*);
};