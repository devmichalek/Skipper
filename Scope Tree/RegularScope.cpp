#include "RegularScope.h"
#include "ConcurrentScope.h"
#include "Console.h"

RegularScope::RegularScope(CommonScope* upNode) : CommonScope(REGULAR)
{
	m_upNode = upNode;
	m_nodes = nullptr;
	m_next = nullptr;
}

RegularScope::~RegularScope()
{
	// ...
}

bool RegularScope::addTask(Command* &cmd, std::string &pathToFile, const char* filename, int &line)
{
	if (m_nodes && m_nodes->m_type == CONCURRENT)
	{
		PrintError(filename, line, "Tasks are not possible inside regular scope with concurrent scopes");
		delete cmd;
		cmd = nullptr;
		pathToFile.clear();
		return false;
	}
	
	return push(cmd, pathToFile, filename, line);
}

bool RegularScope::addScope(CommonScope* newScope, M_TYPE newType, const char* filename, int &line)
{
	if (newType == CONCURRENT && !m_tasks.empty())
	{
		PrintError(filename, line, "Concurrent scopes are not possible inside regular scope with tasks");
		return false;
	}

	if (m_nodes)
	{
		CommonScope* ptrScope = m_nodes;
		while (ptrScope->getNextNode())
			ptrScope = ptrScope->getNextNode();

		if (ptrScope->m_type != newType)
		{
			PrintError(filename, line, "Only one type of nested scope is possible inside regular scope");
			return false;
		}

		if (ptrScope->m_type == REGULAR)
		{
			((RegularScope*)newScope)->m_next = nullptr;
			((RegularScope*)ptrScope)->m_next = (RegularScope*)newScope;
		}	
		else
		{
			((ConcurrentScope*)newScope)->m_next = nullptr;
			((ConcurrentScope*)ptrScope)->m_next = (ConcurrentScope*)newScope;
		}
	}
	else
	{
		m_nodes = newScope;
		m_nodes->m_type = newType;
		if (newType == REGULAR)
			((RegularScope*)m_nodes)->m_next = nullptr;
		else
			((ConcurrentScope*)m_nodes)->m_next = nullptr;
	}

	return true; // success;
}

bool RegularScope::execute()
{
	if (m_nodes)
	{
		if (m_nodes->m_type == REGULAR)
		{	// Regular Scope Type.
			RegularScope* node = (RegularScope*)m_nodes;
			while (node)
			{
				node->execute();
				node = node->m_next;
			}
		}
		else
		{	// Concurrent Scope Type.
			ConcurrentScope* node = (ConcurrentScope*)m_nodes;
			std::vector<std::thread> threads;
			while (node)
			{
				threads.push_back(std::thread([=]() { node->execute(); }));
				node = node->m_next;
			}

			// Wait for all threads to finish.
			std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
		}
	}
	
	if (!m_tasks.empty())
	{
		while (!m_tasks.empty())
		{
			Command* cmd = m_tasks.front();
			if (cmd->parse())
				cmd->run();
			pop();
		}
	}

	return true;
}

void RegularScope::destroy()
{
	if (m_nodes)
	{
		if (m_nodes->m_type == REGULAR)
		{	// Regular Scope Type.
			RegularScope* node = (RegularScope*)m_nodes;
			while (node)
			{
				RegularScope* to_delete = node;
				node = node->m_next;
				to_delete->destroy();
				delete to_delete;
			}
			m_nodes = nullptr;
		}
		else
		{	// Concurrent Scope Type.
			ConcurrentScope* node = (ConcurrentScope*)m_nodes;
			while (node)
			{
				ConcurrentScope* to_delete = node;
				node = node->m_next;
				to_delete->destroy();
				delete to_delete;
			}
			m_nodes = nullptr;
		}
	}
	else if (!m_tasks.empty())
	{
		while (!m_tasks.empty())
		{
			delete m_tasks.front();
			m_tasks.pop();
		}
	}
}

bool RegularScope::capture(RegularScope* &branch, const char* filename, int &line)
{
	if (!branch->m_tasks.empty())
	{	// Branch contains tasks.
		if (m_nodes && m_nodes->m_type == CONCURRENT)
		{
			PrintError(filename, line, "Cannot capture other branch's tasks. This regular scope contains concurrent scopes");
			return false;
		}

		std::string nofile = "";
		while (!m_tasks.empty())
		{
			push(m_tasks.front(), nofile, filename, line); // this is now new owner of branch's tasks
			branch->m_tasks.pop(); // Popping without deleting pointer.
		}
	}
	
	if (branch->m_nodes)
	{	// Branch contains nodes.
		if (branch->m_nodes->m_type == CONCURRENT && !m_tasks.empty())
		{
			PrintError(filename, line, "Cannot capture other branch's concurrent scopes. This regular scope contains tasks");
			return false;
		}
		else if (branch->m_nodes->m_type == CONCURRENT && m_nodes && m_nodes->m_type == REGULAR)
		{
			PrintError(filename, line, "Cannot capture other branch's concurrent scopes. This regular scope contains regular scopes");
			return false;
		}

		// this is now owner of new nodes
		if (m_nodes)
		{
			CommonScope* ptrScope = m_nodes;
			while (ptrScope->getNextNode())
				ptrScope = ptrScope->getNextNode();
			if (ptrScope->m_type == REGULAR)
				((RegularScope*)ptrScope)->m_next = (RegularScope*)branch->m_nodes;
			else
				((ConcurrentScope*)ptrScope)->m_next = (ConcurrentScope*)branch->m_nodes;
		}
		else
			m_nodes = branch->m_nodes;
	}

	branch->m_nodes = nullptr;
	branch->m_next = nullptr;
	delete branch;
	branch = nullptr;
	return true;
}

RegularScope* RegularScope::getNextNode()
{
	return m_next;
}
