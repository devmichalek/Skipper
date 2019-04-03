#include "ConcurrentScope.h"
#include "RegularScope.h"

ConcurrentScope::ConcurrentScope(RegularScope* &upNode) : CommonScope(CONCURRENT)
{
	m_upNode = upNode;
}

ConcurrentScope::~ConcurrentScope()
{
	// concurrent scope is never a global scope
	// ...
}

bool ConcurrentScope::addTask(Command* cmd, std::string pathToFile)
{
	push(cmd, pathToFile);
	return true;
}


bool ConcurrentScope::addScope(CommonScope* newScope, M_TYPE newType)
{
	if (newType == CONCURRENT)
	{	// error, concurrent scope is not possible inside concurrent scope

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
	else
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

ConcurrentScope* ConcurrentScope::getNextNode()
{
	return m_next;
}