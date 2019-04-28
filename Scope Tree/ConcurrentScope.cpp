#include "ConcurrentScope.h"
#include "RegularScope.h"
#include "Console.h"

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

bool ConcurrentScope::addTask(Command* &cmd, Redirection &redirection, const char* filename, int &line)
{
	return push(cmd, redirection, filename, line);
}


bool ConcurrentScope::addScope(CommonScope* newScope, M_TYPE newType, const char* filename, int &line)
{
	if (newType == CONCURRENT)
	{
		PrintError(filename, line, "Concurrent scope cannot be the child of concurrent scope");
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

	++m_children;
	return true;
}

bool ConcurrentScope::execute()
{
	if (!m_tasks.empty())
	{
		while (!m_tasks.empty())
		{
			Command* cmd = m_tasks.front();
			cmd->run();
			pop();
		}
	}

	if (m_nodes)
	{
		RegularScope* node = m_nodes;
		while (node)
		{
			node->execute();
			node = node->m_next;
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
			delete m_tasks.front();
			m_tasks.pop();
		}
	}
}

bool ConcurrentScope::capture(RegularScope* &branch, const char* filename, int &line)
{
	if (!branch->m_tasks.empty())
	{	// Branch contains tasks.
		Redirection noredirection = std::make_pair(std::string(""), RedirectionType::LEFT);
		while (!branch->m_tasks.empty())
		{
			push(branch->m_tasks.front(), noredirection, filename, line); // this scope is now owner of new tasks
			branch->m_tasks.pop(); // popping without deleting pointer
		}
	}

	if (branch->m_nodes)
	{	// Branch contains nodes.
		if (branch->m_nodes->m_type == CONCURRENT)
		{
			PrintError(filename, line, "Cannot capture other branch's concurrent scopes. This concurrent scope can contain only regular scopes");
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
		m_children += branch->m_children;
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