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
		int openRegular = 0;
		int openConcurrent = 0;
		return connect(file, tree, line, openRegular, openConcurrent);
	}
	else
		printf("Cannot open %s file\n", filename);

	return false; // error
}

bool Interpreter::connect(std::fstream &file, CommonScope* &upNode, int &line, int &ore, int &oco)
{
	char c;
	CommonScope* ptr = nullptr;
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
		
		if (c == EOF)
			break;
		else if (c == '#')
		{	// skip comment
			while (c = file.get())
			{
				if (c == '\n')
				{
					++line;
					break;
				}
				else if (c == EOF)
					break;
			}
			continue;
		}
		else if (c == '{')
		{	// new regular scope
			++ore;
			ptr = new RegularScope(upNode);
			upNode->addScope(ptr, CommonScope::REGULAR, line);
			if (!connect(file, ptr, line, ore, oco))
				return false;
		}
		else if (c == '}')
		{
			if (ore == 0)
			{
				printf("Error: Syntax error too many closing brackets } at %d line\n", line);
				return false;
			}

			--ore;
			return true;
		}
		else if (c == '[')
		{	// new concurrent scope
			++oco;
			RegularScope* up = dynamic_cast<RegularScope*>(upNode);
			ptr = new ConcurrentScope(up);
			upNode->addScope(ptr, CommonScope::CONCURRENT, line);
			if (!connect(file, ptr, line, ore, oco))
				return false;
		}
		else if (c == ']')
		{
			if (!oco)
			{
				printf("Error: Syntax error too many closing brackets ] at %d line\n", line);
				return false;
			}

			--oco;
			return true;
		}
		else
		{	// new task
			std::string sline = "";
			sline += c;
			while (c = file.get())
			{
				if (c == '\n' || c == EOF)
					break;
				sline += c;
			}

			{	// remove whitespaces and comment
				size_t pos = sline.rfind('#');
				if (pos != std::string::npos)
					sline = sline.substr(0, pos);
				int j = sline.size();
				for (int i = sline.size() - 1; i >= 0; --i)
				{
					if (sline[i] == '\f' ||
						sline[i] == '\r' ||
						sline[i] == '\t' ||
						sline[i] == '\v' ||
						sline[i] == ' ')
						j = i;
					else
						break;
				}
				sline = sline.substr(0, j);
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

	if (ore || oco)
	{
		printf("Error: Number of opening brackets and closing brackets is different, %d line\n", line);
		return false;
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
	{
		printf("Error: Yacc message is corrupted");
		return;
	}

	std::string ref = *msg;
	ref += " ";
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