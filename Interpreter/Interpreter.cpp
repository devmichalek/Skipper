#include "Interpreter.h"
#include "Console.h"
#include "Parser.h"
#include "cmd_compare.h"
#include "cmd_copy.h"
#include "cmd_echo.h"
#include "cmd_help.h"
#include "cmd_include.h"
#include "cmd_list.h"
#include "cmd_move.h"
#include "cmd_regex.h"
#include "cmd_remove.h"
#include "cmd_rename.h"
#include "cmd_run.h"
#include "cmd_search.h"
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

	m_redirection = std::make_pair(std::string(""), RedirectionType::LEFT);
	m_sFileName = "";
	m_sCaughtMsg = "";
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
		if (!status) {
			PrintError(parentfile ? parentfile : filename, linef, "Interpreter scanning process failed");
			m_bError = true;
		}
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
			int fline = 0;
			while (sign = file.get())
			{
				if (sign == '\n') {
					++fline;
					break;
				}
				if(sign == EOF)
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
				if (!m_pCmd->parse(m_sFileName.c_str(), line))	// Error Parse Command.
				{
					PrintError(m_sFileName.c_str(), line, "Caught error while parsing command");
					return false;
				}
				else
				{
					std::string cmd_ident = std::string("Correctly inserted and parsed new command ") + std::string(HandlerIdentTable[(int)m_pCmd->handler()]);
					PrintSuccess(m_sFileName.c_str(), line, cmd_ident.c_str());
				}

				if (m_pCmd->handler() == Handler::CMD_INCLUDE)
				{	// Command evaluated during static interpretation!
					if (m_pCmd->m_global_buffer.empty())
					{
						if (!upNode->addTask(m_pCmd, m_redirection, m_sFileName.c_str(), line))
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
				else
				{
					if (!upNode->addTask(m_pCmd, m_redirection, m_sFileName.c_str(), line))
						return false;
				}

				m_pCmd = nullptr;
			}
			else if (m_bError)
			{
				if (m_sCaughtMsg.empty()) {
					printf(" (flex)\n");
					m_sCaughtMsg = "caught message is empty";
				}

				PrintError(m_sFileName.c_str(), line, m_sCaughtMsg.c_str());
				return false;
			}

			line += fline;
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
void Interpreter::analyze()
{
	Interpreter* ptr = this;
	while (ptr->m_child)
		ptr = ptr->m_child;

	Command* &pCmd = ptr->m_pCmd;
	bool &bError = ptr->m_bError;
	std::string &sCaughtStr = ptr->m_sCaughtMsg;
	pCmd = nullptr;

	if (m_svSwitches.subtypes.back() == Subtype::REDIRECTION)
	{	// Error.
		sCaughtStr = "Missing <file name> for previously defined redirection \"" + m_svSwitches.switches.back() + "\"";
		m_bError = true;
	}
	else
	{
		{	// Redirection.
			auto it = std::find(m_svSwitches.subtypes.begin(), m_svSwitches.subtypes.end(), Subtype::REDIRECTION);
			if (it != m_svSwitches.subtypes.end())
			{
				int distance = (int)std::distance(m_svSwitches.subtypes.begin(), it);
				std::string redirection = m_svSwitches.switches[distance];
				std::string filename = m_svSwitches.switches[distance + 1];

				// Erase last two items.
				m_svSwitches.switches.erase(m_svSwitches.switches.begin() + distance, m_svSwitches.switches.end());
				m_svSwitches.subtypes.erase(m_svSwitches.subtypes.begin() + distance, m_svSwitches.subtypes.end());

				if (redirection == ">")
					m_redirection = std::make_pair(filename, RedirectionType::RIGHT);
				else if (redirection == ">>")
					m_redirection = std::make_pair(filename, RedirectionType::RIGHT_ADD);
				else
				{
					if (redirection == "<")
					{	// Overwrite. Clear but keep command name.
						m_svSwitches.switches.erase(m_svSwitches.switches.begin() + 1, m_svSwitches.switches.end());
						m_svSwitches.subtypes.erase(m_svSwitches.subtypes.begin() + 1, m_svSwitches.subtypes.end());
					}

					// Add new switches.
					// Add better implementation here.
					std::ifstream redirectionfile;
					redirectionfile.open(filename);
					if (!redirectionfile.is_open())
					{	// Error.
						sCaughtStr = "Cannot open redirection file \"" + filename + "\"";
						m_bError = true;
						m_svSwitches.clear();
						return;
					}
					else
					{	// Insert switches from file.
						std::string line;
						std::getline(redirectionfile, line);
						std::vector<std::string> addSwitches = extract(line, 0);
						m_svSwitches.switches.insert(m_svSwitches.switches.end(), addSwitches.begin(), addSwitches.end());
					}
				}
			}
			else
				m_redirection = std::make_pair(std::string(""), RedirectionType::LEFT);
		}

		std::string ref = m_svSwitches.switches.front(); // Comand Name.
		m_svSwitches.switches.erase(m_svSwitches.switches.begin());
		m_svSwitches.subtypes.erase(m_svSwitches.subtypes.begin());

		if (ref.front() != ' ')
			ref += " ";
		if (ref[0] == '!')
		{
			if (ref[1] == 'c')
			{
				if (ref.substr(2, 7) == "ompare ")
				{	// compare
					pCmd = new Command_Compare(m_svSwitches.switches);
				}
				else if (ref.substr(2, 4) == "opy ")
				{	// copy
					pCmd = new Command_Copy(m_svSwitches.switches);
				}
			}
			else if (ref[1] == 'e')
			{
				if (ref.substr(2, 4) == "cho ")
				{	// echo
					pCmd = new Command_Echo(m_svSwitches.switches);
				}
			}
			else if (ref[1] == 'h')
			{
				if (ref.substr(2, 4) == "elp ")
				{	// help
					pCmd = new Command_Help(m_svSwitches.switches);
				}
			}
			else if (ref[1] == 'i')
			{
				if (ref.substr(2, 7) == "nclude ")
				{	// include
					pCmd = new Command_Include(m_svSwitches.switches);
				}
			}
			else if (ref[1] == 'l')
			{
				if (ref.substr(2, 4) == "ist ")
				{	// list
					pCmd = new Command_List(m_svSwitches.switches);
				}
			}
			else if (ref[1] == 'm')
			{
				if (ref.substr(2, 4) == "ove ")
				{	// move
					pCmd = new Command_Move(m_svSwitches.switches);
				}
			}
			else if (ref[1] == 'r')
			{
				if (ref.substr(2, 5) == "egex ")
				{	// regex
					pCmd = new Command_Regex(m_svSwitches.switches);
				}
				else if (ref.substr(2, 6) == "emove ")
				{	// remove
					pCmd = new Command_Remove(m_svSwitches.switches);
				}
				else if (ref.substr(2, 6) == "ename ")
				{	// rename
					pCmd = new Command_Rename(m_svSwitches.switches);
				}
				else if (ref.substr(2, 3) == "un ")
				{	// run
					pCmd = new Command_Run(m_svSwitches.switches);
				}
			}
			else if (ref[1] == 's')
			{
				if (ref.substr(2, 6) == "earch ")
				{	// search
					pCmd = new Command_Search(m_svSwitches.switches);
				}
			}
			else if (ref[1] == 'w')
			{
				if (ref.substr(2, 4) == "ait ")
				{	// wait
					pCmd = new Command_Wait(m_svSwitches.switches);
				}
				else if (ref.substr(2, 4) == "ipe ")
				{	// wipe
					pCmd = new Command_Wipe(m_svSwitches.switches);
				}
			}
		}

		if (!pCmd)
		{
			sCaughtStr = "Command " + ref + " not found";
			bError = true;
		}
	}

	m_svSwitches.clear();
}