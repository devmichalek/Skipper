#include "RegularScope.h"
#include "ConcurrentScope.h"

RegularScope::RegularScope(CommonScope* upNode) : CommonScope(REGULAR)
{
	m_upNode = upNode;
	m_nodes = nullptr;
	m_next = nullptr;
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

bool RegularScope::addTask(Command* &cmd, std::string &pathToFile, int &line)
{
	if (m_nodes && m_nodes->m_type == CONCURRENT)
	{
		printf("Error: Tasks are not possible inside regular scope with concurrent scopes %d line\n", line);
		delete cmd;
		cmd = nullptr;
		pathToFile.clear();
		return false;
	}
	
	return push(cmd, pathToFile, line);
}

bool RegularScope::addScope(CommonScope* newScope, M_TYPE newType, int &line)
{
	if (newType == CONCURRENT && !m_tasks.empty())
	{
		printf("Error: Concurrent scopes are not possible inside regular scope with tasks %d line\n", line);
		return false;
	}

	if (m_nodes)
	{
		CommonScope* ptrScope = m_nodes;
		while (ptrScope->getNextNode())
			ptrScope = ptrScope->getNextNode();

		if (ptrScope->m_type != newType)
		{
			printf("Error: Only one type of nested scope is possible inside regular scope %d line\n", line);
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
		{	// regular
			RegularScope* node = (RegularScope*)m_nodes;
			while (node)
			{
				node->execute();
				node = node->m_next;
			}
		}
		else
		{	// concurrent
			ConcurrentScope* node = (ConcurrentScope*)m_nodes;
			std::vector<std::thread> threads;
			while (node)
			{
				threads.push_back(std::thread([&]() { node->execute(); }));
				node = node->m_next;
			}

			// wait for all threads to finish
			// temporary solution
			std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
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

void RegularScope::destroy()
{
	if (m_nodes)
	{
		if (m_nodes->m_type == REGULAR)
		{	// regular
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
		{	// concurrent
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
			delete m_tasks.front().first;
			m_tasks.pop();
		}
	}
}

bool RegularScope::capture(RegularScope* &branch, int &line)
{
	if (!branch->m_tasks.empty())
	{	// branch has tasks
		if (m_nodes && m_nodes->m_type == CONCURRENT)
		{
			printf("Error: Cannot capture other branch's tasks. This regular scope contains concurrent scopes, line %d\n", line);
			return false;
		}

		std::string nofile = "";
		while (!m_tasks.empty())
		{
			push(m_tasks.front().first, nofile, line); // this scope is now owner of new tasks
			m_tasks.pop(); // popping without deleting pointer
		}
	}
	
	if (branch->m_nodes)
	{	// branch has nodes
		if (branch->m_nodes->m_type == CONCURRENT && !m_tasks.empty())
		{
			printf("Error: Cannot capture other branch's concurrent scopes. This regular scope contains tasks, line %d\n", line);
			return false;
		}
		else if (branch->m_nodes->m_type == CONCURRENT && m_nodes && m_nodes->m_type == REGULAR)
		{
			printf("Error: Cannot capture other branch's concurrent scopes. This regular scope contains regular scopes, line %d\n", line);
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
