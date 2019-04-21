#include "Interpreter.h"
#include "Console.h"
#include "Parser.h"
#include "cmd_compare.h"
#include "cmd_copy.h"
#include "cmd_help.h"
#include "cmd_include.h"
#include "cmd_list.h"
#include "cmd_move.h"
#include "cmd_regex.h"
#include "cmd_remove.h"
#include "cmd_rename.h"
#include "cmd_wait.h"
#include "cmd_wipe.h"

Interpreter::Interpreter(RegularScope* &parent)
{
	m_bError = false;
	m_pCmd = nullptr;
	m_child = nullptr;

	m_pTree = new RegularScope(parent);
	if (parent) {
		int line = -1;
		parent->addScope((CommonScope*)m_pTree, CommonScope::REGULAR, "", line);
	}

	m_sPathToFile = "";
	m_sFileName = "";
	m_sCatchedMsg = "";
}

Interpreter::~Interpreter()
{
	if (m_pTree)
	{
		m_pTree->destroy();
		delete m_pTree;
	}
}

bool Interpreter::scan(const char* filename, const char* parentfile, int linef)
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
			PrintError(parentfile ? parentfile : filename, linef, "Interpreter scanning process failed");
		return status;
	}
	else
	{
		std::string buf = "Cannot open " + std::string(filename) + " file";
		PrintError(parentfile, linef, buf.c_str());
	}

	return false; // error
}

bool Interpreter::connect(std::fstream &file, CommonScope* &upNode, int &line, int &ore, int &oco)
{
	while (char sign = file.get())
	{
		// Skip whitespaces.
		while (true)
		{
			if (sign == '\n')
				++line;
			else if (sign != '\f' &&
				sign != '\r' &&
				sign != '\t' &&
				sign != '\v' &&
				sign != ' ')
				break;
			sign = file.get();
		}
		
		// End of file, break.
		if (sign == EOF)
			break; 
		
		 // Skip comment.
		else if (sign == '#')
		{
			while (sign = file.get())
			{
				if (sign == '\n')
				{
					++line;
					break;
				}
				else if (sign == EOF)
					break;
			}
			continue;
		}

		// Open Regular Scope.
		else if (sign == '{')
		{
			++ore;
			CommonScope* ptr = new RegularScope(upNode);
			upNode->addScope(ptr, CommonScope::REGULAR, m_sFileName.c_str(), line);
			if (!connect(file, ptr, line, ore, oco))
			{	// Destroy if failed.
				ptr->destroy();
				return false;
			}
		}

		// Close Regular Scope
		else if (sign == '}')
		{
			if (!ore)
			{
				PrintError(m_sFileName.c_str(), line, "Syntax error, too many closing brackets '}'");
				return false;
			}

			--ore;
			return true;
		}

		// Open Concurrent Scope
		else if (sign == '[')
		{
			++oco;
			RegularScope* up = (RegularScope*)(upNode);
			CommonScope* ptr = new ConcurrentScope(up);
			upNode->addScope(ptr, CommonScope::CONCURRENT, m_sFileName.c_str(), line);
			if (!connect(file, ptr, line, ore, oco))
			{	// Destroy if failed.
				ptr->destroy();
				return false;
			}
		}

		// Close Concurrent Scope
		else if (sign == ']')
		{
			if (!oco)
			{
				PrintError(m_sFileName.c_str(), line, "Syntax error, too many closing brackets ']'");
				return false;
			}

			--oco;
			return true;
		}

		// Add new task.
		else
		{	// Prepare string.
			std::string sline("");
			sline += sign;
			while (sign = file.get())
			{
				if (sign == '\n' || sign == EOF)
					break;
				sline += sign;
			}

			{	// Remove comment.
				size_t pos = sline.rfind('#');
				if (pos != std::string::npos)
					sline = sline.substr(0, pos);
				// Cut whitespaces.
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

			// Send string to Yacc
			sline += "\n";
			parse(sline.c_str());

			// Retrieve.
			if (m_pCmd)
			{
				std::string cmd_ident = std::string("Correctly inserted new command ") + std::string(HandlerIdentTable[(int)m_pCmd->handler()]);
				PrintSuccess(m_sFileName.c_str(), line, cmd_ident.c_str());
				if (m_pCmd->handler() == Handler::CMD_INCLUDE)
				{	// Command evaluated during static interpretation!
					if (m_pCmd->parse() && !m_pCmd->run())
					{
						if (m_pCmd->m_global_buffer.empty())
						{
							if (!upNode->addTask(m_pCmd, m_sPathToFile, m_sFileName.c_str(), line))
								return false;
						}
						else
						{
							RegularScope* regscope = nullptr;
							Interpreter inter(regscope);
							m_child = &inter;
							bool status = inter.scan(m_pCmd->m_global_buffer.c_str(), m_sFileName.c_str(), line);
							if (!status)
							{
								m_child = nullptr;
								return false;
							}

							if (upNode->m_type == CommonScope::REGULAR)
								status = ((RegularScope*)upNode)->capture(inter.m_pTree, m_sFileName.c_str(), line);
							else
								status = ((ConcurrentScope*)upNode)->capture(inter.m_pTree, m_sFileName.c_str(), line);
							
							m_child = nullptr;
							if (!status)
								return false;
						}
					}
				}
				else
				{
					if (!upNode->addTask(m_pCmd, m_sPathToFile, m_sFileName.c_str(), line))
						return false;
				}
			}
			else if (m_bError)
			{
				if (m_sCatchedMsg.empty()) {
					printf(" (Flex)\n");
					m_sCatchedMsg = "catched msg is empty";
				}

				PrintError(m_sFileName.c_str(), line, m_sCatchedMsg.c_str());
				return false;
			}
		}
	}

	if (ore || oco)
	{
		PrintError(m_sFileName.c_str(), line, "Number of opening brackets and closing brackets is not the same");
		return false;
	}

	// Success.
	return true;
}

// Yacc calls this function if pattern is found.
void Interpreter::analyze(std::string* msg)
{
	Interpreter* ptr = this;
	while (ptr->m_child)
		ptr = ptr->m_child;

	Command* &pCmd = ptr->m_pCmd;
	bool &bError = ptr->m_bError;
	std::string &sCatchedStr = ptr->m_sCatchedMsg;
	pCmd = nullptr;

	if (bError)
	{
		if (msg)
			delete msg;
		bError = false;
		return;
	}

	if (!msg)
	{
		sCatchedStr = "Yacc message is corrupted";
		bError = true;
		return;
	}

	std::string ref = *msg;
	{	// Extract file name (if exists)
		size_t pos = ref.rfind('>'); // rfind
		if (pos != std::string::npos)
		{
			m_sPathToFile = ref.substr(++pos, ref.size() - pos);
			pos = ref.find('>'); // find
			ref.erase(pos, ref.size() - pos);
		}
	}

	if (ref.front() != ' ')
		ref += " ";
	if (ref[0] == '!')
	{
		if (ref[1] == 'c')
		{
			if (ref.substr(2, 7) == "ompare ")
			{	// compare
				pCmd = new Command_Compare(extract(ref, 9));
			}
			else if (ref.substr(2, 4) == "opy ")
			{	// copy
				pCmd = new Command_Copy(extract(ref, 6));
			}
		}
		else if (ref[1] == 'h')
		{
			if (ref.substr(2, 4) == "elp ")
			{	// help
				pCmd = new Command_Help(extract(ref, 6));
			}
		}
		else if (ref[1] == 'i')
		{
			if (ref.substr(2, 7) == "nclude ")
			{	// include
				pCmd = new Command_Include(extract(ref, 9));
			}
		}
		else if (ref[1] == 'l')
		{
			if (ref.substr(2, 4) == "ist ")
			{	// list
				pCmd = new Command_List(extract(ref, 6));
			}
		}
		else if (ref[1] == 'm')
		{
			if (ref.substr(2, 4) == "ove ")
			{	// move
				pCmd = new Command_Move(extract(ref, 6));
			}
		}
		else if (ref[1] == 'r')
		{
			if (ref.substr(2, 5) == "egex ")
			{	// regex
				pCmd = new Command_Regex(extract(ref, 7));
			}
			else if (ref.substr(2, 6) == "emove ")
			{	// remove
				pCmd = new Command_Remove(extract(ref, 8));
			}
		}
		else if (ref[1] == 'w')
		{
			if (ref.substr(2, 4) == "ait ")
			{	// wait
				pCmd = new Command_Wait(extract(ref, 6));
			}
			else if (ref.substr(2, 4) == "ipe ")
			{	// wipe
				pCmd = new Command_Wait(extract(ref, 6));
			}
			else if (ref.substr(2, 6) == "ename ")
			{	// rename
				pCmd = new Command_Rename(extract(ref, 8));
			}
		}
	}

	if (!pCmd)
	{
		sCatchedStr = "Command " + *msg + " not found";
		bError = true;
	}

	if (msg)
		delete msg;
}