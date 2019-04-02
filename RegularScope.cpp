#include "RegularScope.h"
#include "ConcurrentScope.h"

RegularScope::RegularScope(CommonScope* upNode) : CommonScope(REGULAR)
{
	m_upNode = upNode;
}

RegularScope::~RegularScope()
{
	if (!m_upNode && !m_rofiles.empty())
	{	// only global scope can access this code
		for (auto it = m_rofiles.begin(); it != m_rofiles.end(); ++it)
		{
			if ((*it).first->is_open())
				(*it).first->close();
			delete (*it).first;
		}
		m_rofiles.clear();
	}
}

bool RegularScope::addTask(Command* cmd, std::string pathToFile)
{
	if (m_nodes && m_nodes->m_type == CONCURRENT)
	{	// error, tasks are not possible inside regular scope with concurrent scope

		return false;
	}

	push(cmd, pathToFile);
	return true;
}

bool RegularScope::addScope(CommonScope* newScope, M_TYPE newType)
{
	if (newType == CONCURRENT && !m_tasks.empty())
	{	// error, concurrent scope is not possible inside regular scope with tasks

		return false;
	}

	if (m_nodes)
	{
		CommonScope* ptrScope = m_nodes;
		while (ptrScope->getNextNode())
			ptrScope = ptrScope->getNextNode();

		if (ptrScope->m_type != newType)
		{	// error, only one type possible

			return false;
		}

		if (ptrScope->m_type == REGULAR)
		{
			((RegularScope*)newScope)->m_nextNode = nullptr;
			((RegularScope*)ptrScope)->m_nextNode = (RegularScope*)newScope;
		}	
		else
		{
			((ConcurrentScope*)newScope)->m_nextNode = nullptr;
			((ConcurrentScope*)ptrScope)->m_nextNode = (ConcurrentScope*)newScope;
		}
	}
	else
	{
		m_nodes = newScope;
		m_nodes->m_type = newType;
		if (newType == REGULAR)
			((RegularScope*)m_nodes)->m_nextNode = nullptr;
		else
			((ConcurrentScope*)m_nodes)->m_nextNode = nullptr;
	}

	return true; // success;
}

bool RegularScope::execute()
{

	return true;
}

RegularScope* RegularScope::getNextNode()
{
	return m_nextNode;
}
