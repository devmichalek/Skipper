#include "Interpreter.h"
#include "parser.h"
#include "cmd_help.h"
#include "cmd_include.h"
#include "cmd_list.h"
#include "cmd_regex.h"
#include "cmd_remove.h"

Interpreter::Interpreter()
{
	m_bExit = false;
	m_bError = false;
	m_pCmd = nullptr;
	m_pTree = nullptr;
	m_sPathToFile = "";
}

bool Interpreter::scan(const char* filename)
{
	std::fstream file;
	file.open(filename, std::ios::in);
	if (file.is_open())
	{
		m_pTree = new RegularScope(nullptr);
		CommonScope* tree = m_pTree;
		int line = 1;
		return connect(file, tree, line);
	}
	else
		printf("Cannot open %s file\n", filename);

	return false; // error
}

bool Interpreter::connect(std::fstream &file, CommonScope* &upNode, int &line)
{
	char c;
	CommonScope* ptr = nullptr;
	bool openRegular = false;
	bool openConcurrent = false;

	while (c = file.get())
	{
		while (true)
		{	// skip whitespaces
			if (c == '\n')
				++line;
			else if (c != '\f' &&
				c != '\r' &&
				c != '\t' &&
				c != '\v' &&
				c != ' ')
				break;
			c = file.get();
		}
	
		if (c == '{')
		{	// new regular scope
			if (openRegular)
			{
				printf("Error: Syntax error missing closing bracket } at %d line\n", line);
				return false;
			}

			openRegular = true;
			ptr = new RegularScope(upNode);
			upNode->addScope(ptr, CommonScope::REGULAR, line);
			if (!connect(file, ptr, line))
				return false;
		}
		else if (c == '}')
		{
			if (!openRegular)
			{
				printf("Error: Syntax error too many closing brackets } at %d line\n", line);
				return false;
			}

			openRegular = false;
			return true;
		}
		else if (c == '[')
		{	// new concurrent scope
			if (openConcurrent)
			{
				printf("Error: Syntax error missing closing bracket ] at %d line\n", line);
				return false;
			}

			openConcurrent = true;
			RegularScope* up = dynamic_cast<RegularScope*>(upNode);
			ptr = new ConcurrentScope(up);
			upNode->addScope(ptr, CommonScope::CONCURRENT, line);
			if (!connect(file, ptr, line))
				return false;
		}
		else if (c == ']')
		{
			if (!openConcurrent)
			{
				printf("Error: Syntax error too many closing brackets ] at %d line\n", line);
				return false;
			}

			openConcurrent = false;
			return true;
		}
		else
		{	// new task
			std::string sline = "";
			sline += c;
			while (c = file.get())
			{
				if (c == '\n')
					break;
				else if (c == '}' || c == ']')
				{
					std::streamoff off = -1;
					file.seekg(off);
					break;
				}
				sline += c;
			}

			sline += "\n";
			parse(sline.c_str());
			if (m_pCmd)
			{
				if (!upNode->addTask(m_pCmd, m_sPathToFile, line))
					return false;
				m_pCmd = nullptr;
				m_sPathToFile = "";
			}
			else if (m_bError)
			{
				printf(" at %d line\n", line);
				return false;
			}
		}
	}

	return true; // ok
}

void Interpreter::exit()
{
	printf("Terminating...\n");
	m_bExit = true;
}

// Yacc calls this function if pattern is found.
void Interpreter::analyze(std::string* msg)
{
	m_pCmd = nullptr;

	if (m_bError)
	{
		if (msg)
			delete msg;
		m_bError = false;
		return;
	}

	if (!msg)
		return;

	std::string &ref = (*msg);
	if (ref[0] == '!')
	{
		if (ref[1] == 'l')
		{
			if (ref.substr(2, 4) == "ist ")
			{	// list
				m_pCmd = new Command_List(extract(ref, 6));
			}
		}
		else if (ref[1] == 'r')
		{
			if (ref[2] == 'e')
			{
				if (ref.substr(3, 4) == "gex ")
				{	// regex
					m_pCmd = new Command_Regex(extract(ref, 7));
				}
				else if (ref.substr(3, 5) == "move ")
				{	// remove
					m_pCmd = new Command_Remove(extract(ref, 8));
				}
			}
		}
	}

	if (!m_pCmd)
	{
		printf("Error: Command %s not found", msg->c_str());
		m_bError = true;
	}

	if (msg)
		delete msg;
}