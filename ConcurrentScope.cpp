#include "ConcurrentScope.h"
#include "RegularScope.h"

ConcurrentScope::ConcurrentScope(RegularScope* upNode) : CommonScope(CONCURRENT)
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

		((RegularScope*)newScope)->m_nextNode = nullptr;
		((RegularScope*)ptrScope)->m_nextNode = (RegularScope*)newScope;
	}
	else
	{
		m_nodes = (RegularScope*)newScope;
		m_nodes->m_type = newType;
		((RegularScope*)m_nodes)->m_nextNode = nullptr;
	}

	return true; // success;
}

bool ConcurrentScope::execute()
{

	return true;
}

ConcurrentScope* ConcurrentScope::getNextNode()
{
	return m_nextNode;
}