#include "ConcurrentScope.h"
#include "RegularScope.h"

ConcurrentScope::ConcurrentScope(RegularScope* &upNode) : CommonScope(CONCURRENT)
{
	m_upNode = upNode;
	m_nodes = nullptr;
	m_next = nullptr;
}

ConcurrentScope::~ConcurrentScope()
{
	// concurrent scope is never a global scope
	// ...
}

bool ConcurrentScope::addTask(Command* &cmd, std::string &pathToFile, int &line)
{
	return push(cmd, pathToFile, line);
}


bool ConcurrentScope::addScope(CommonScope* newScope, M_TYPE newType, int &line)
{
	if (newType == CONCURRENT)
	{
		printf("Error: Concurrent scope cannot be the child of concurrent scope %d line\n", line);
		return false;
	}

	if (m_nodes)
	{
		RegularScope* ptrScope = m_nodes;
		while (ptrScope->getNextNode())
			ptrScope = ptrScope->getNextNode();

		((RegularScope*)newScope)->m_next = nullptr;
		((RegularScope*)ptrScope)->m_next = (RegularScope*)newScope;
	}
	else
	{
		m_nodes = (RegularScope*)newScope;
		m_nodes->m_type = newType;
		((RegularScope*)m_nodes)->m_next = nullptr;
	}

	return true; // success;
}

bool ConcurrentScope::execute()
{
	if (m_nodes)
	{
		RegularScope* node = m_nodes;
		while (node)
		{
			node->execute();
			node = node->m_next;
		}
	}
	else if (!m_tasks.empty())
	{
		while (!m_tasks.empty())
		{
			Command* cmd = m_tasks.front().first;
			void* fun = m_tasks.front().second;
			cmd->force(fun);
			if (cmd->parse())
				cmd->run();
			pop();
		}
	}

	return true;
}

void ConcurrentScope::destroy()
{
	if (m_nodes)
	{
		RegularScope* node = m_nodes;
		while (node)
		{
			RegularScope* to_delete = node;
			node = node->m_next;
			to_delete->destroy();
			delete to_delete;
		}
		m_nodes = nullptr;
	}
	else if (!m_tasks.empty())
	{
		while (!m_tasks.empty())
		{
			delete m_tasks.front().first;
			m_tasks.pop();
		}
	}
}

bool ConcurrentScope::capture(RegularScope* &branch, int &line)
{
	if (!branch->m_tasks.empty())
	{	// branch has tasks
		std::string nofile = "";
		while (!m_tasks.empty())
		{
			push(m_tasks.front().first, nofile, line); // this scope is now owner of new tasks
			m_tasks.pop(); // popping without deleting pointer
		}
	}

	if (branch->m_nodes)
	{	// branch has nodes
		if (branch->m_nodes->m_type == CONCURRENT)
		{
			printf("Error: Cannot capture other branch's concurrent scopes. This concurrent scope may contain only regular scopes, line %d\n", line);
			return false;
		}

		// this is now owner of new nodes
		if (m_nodes)
		{
			CommonScope* ptrScope = m_nodes;
			while (ptrScope->getNextNode())
				ptrScope = ptrScope->getNextNode();
			((RegularScope*)ptrScope)->m_next = (RegularScope*)branch->m_nodes;
		}
		else
			m_nodes = (RegularScope*)branch->m_nodes;
	}

	branch->m_nodes = nullptr;
	branch->m_next = nullptr;
	delete branch;
	branch = nullptr;
	return true;
}

ConcurrentScope* ConcurrentScope::getNextNode()
{
	return m_next;
}