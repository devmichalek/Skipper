#pragma once
#include "cmd.h"
#include "CommonScope.h"
#include "ConcurrentScope.h"
#include "RegularScope.h"
#include "Arguments.h"
#include "Redirection.h"

#include <limits>
#include <fstream>
#include <iostream>

class Interpreter final
{
	Command* m_pCmd;
	Interpreter* m_child;
	Redirection m_redirection;
public:
	bool m_bError;
	std::string m_sCaughtMsg;
	std::string m_sFileName;
	RegularScope* m_pTree;
	Arguments m_svSwitches;

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
	void analyze();
};