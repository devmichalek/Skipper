#include "Interpreter.h"
#include "Parser.h"
#include "cmd_help.h"
#include "cmd_include.h"
#include "cmd_list.h"
#include "cmd_regex.h"
#include "cmd_remove.h"

Interpreter::Interpreter(RegularScope* &parent)
{
	m_bExit = false;
	m_bError = false;
	m_pCmd.reset(nullptr);
	m_child = nullptr;
	m_pTree = new RegularScope(parent);
	m_sPathToFile = "";
}

Interpreter::~Interpreter()
{
	if (m_pTree)
	{
		m_pTree->destroy();
		delete m_pTree;
	}
}

bool Interpreter::scan(const char* filename, const char* parentfile, int line)
{
	std::fstream file;
	file.open(filename, std::ios::in);
	if (file.is_open())
	{
		m_sFileName = filename;
		CommonScope* tree = m_pTree;
		int line = 1;
		int openRegular = 0;
		int openConcurrent = 0;
		bool status = connect(file, tree, line, openRegular, openConcurrent);
		if (!status)
			printf("Error: Interpreter scanning process failed\n");
		return status;
	}
	else
	{
		if (parentfile && line != -1)
			printf("Error: Cannot open %s file, %s line %d\n", filename, parentfile, line);
		else
			printf("Error: Cannot open %s file\n", filename);
	}

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
			{
				ptr->destroy();
				return false;
			}
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
			{
				ptr->destroy();
				return false;
			}
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
				size_t j = sline.size();
				for (int i = (int)sline.size() - 1; i >= 0; --i)
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
				if (m_pCmd->handler() == Handler::CMD_INCLUDE)
				{	// Command evaluated during static interpretation!
					if (m_pCmd->parse() && !m_pCmd->run())
					{
						if (m_pCmd->m_global_buffer.empty())
						{
							Command* temp = m_pCmd.release();
							if (!upNode->addTask(temp, m_sPathToFile, line))
								return false;
						}
						else
						{
							RegularScope* regscope =  new RegularScope(nullptr);
							Interpreter inter(regscope);
							m_child = &inter;
							bool status = inter.scan(m_pCmd->m_global_buffer.c_str(), m_sFileName.c_str(), line);
							if (!status)
							{
								regscope->destroy();
								delete regscope;
								regscope = nullptr;
								m_child = nullptr;
								return false;
							}

							if (upNode->m_type == CommonScope::REGULAR)
								status = ((RegularScope*)upNode)->capture(regscope, line);
							else
								status = ((ConcurrentScope*)upNode)->capture(regscope, line);

							if (regscope)
							{
								regscope->destroy();
								delete regscope;
								regscope = nullptr;
							}
							
							m_child = nullptr;
							if (!status)
								return false;
						}
					}
				}
				else
				{
					Command* temp = m_pCmd.release();
					if (!upNode->addTask(temp, m_sPathToFile, line))
						return false;
				}
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
	Interpreter* ptrInterpreter = this;
	while (ptrInterpreter->m_child)
		ptrInterpreter = ptrInterpreter->m_child;

	std::unique_ptr<Command> &command = ptrInterpreter->m_pCmd;
	command.reset(nullptr);

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
		if (ref[1] == 'i')
		{
			if (ref.substr(2, 7) == "nclude ")
			{	// include
				command.reset(new Command_Include(extract(ref, 9)));
			}
		}
		else if (ref[1] == 'l')
		{
			if (ref.substr(2, 4) == "ist ")
			{	// list
				command.reset(new Command_List(extract(ref, 6)));
			}
		}
		else if (ref[1] == 'r')
		{
			if (ref.substr(2, 5) == "egex ")
			{	// regex
				command.reset(new Command_Regex(extract(ref, 7)));
			}
			else if (ref.substr(2, 6) == "emove ")
			{	// remove
				command.reset(new Command_Remove(extract(ref, 8)));
			}
		}
	}

	if (!command)
	{
		printf("Error: Command %s not found", msg->c_str());
		m_bError = true;
	}

	if (msg)
		delete msg;
}