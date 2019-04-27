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

bool RegularScope::addTask(Command* &cmd, Redirection &redirection, const char* filename, int &line)
{
	if (m_nodes && m_nodes->m_type == CONCURRENT)
	{
		PrintError(filename, line, "Tasks are not possible inside regular scope with concurrent scopes");
		delete cmd;
		cmd = nullptr;
		redirection = std::make_pair(std::string(""), RedirectionType::LEFT);
		return false;
	}
	
	return push(cmd, redirection, filename, line);
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

	++m_children;
	return true; // success;
}

bool RegularScope::execute()
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

		Redirection noredirection = std::make_pair(std::string(""), RedirectionType::LEFT);
		while (!branch->m_tasks.empty())
		{
			push(branch->m_tasks.front(), noredirection, filename, line); // this is now new owner of branch's tasks
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

void RegularScope::consolidate()
{
	// If there is one child merge.
	// { {} } -> {}
	// { [] } -> {}

	// Go to the bottom
	if (m_children)
	{
		CommonScope* ptr = m_nodes;
		while (ptr)
		{
			ptr->consolidate();
			ptr = ptr->getNextNode();
		}
	}

	if (m_children == 1 && m_nodes->m_type == REGULAR)
	{	// Child nodes and tasks are now parent's nodes and tasks.
		if (m_nodes->m_type == CONCURRENT)
		{
			ConcurrentScope* ptr = (ConcurrentScope*)m_nodes;
			if (!ptr->m_tasks.empty())
			{
				int line = -1;
				Redirection noredirection = std::make_pair(std::string(""), RedirectionType::LEFT);
				while (!ptr->m_tasks.empty())
				{
					push(ptr->m_tasks.front(), noredirection, nullptr, line);
					ptr->m_tasks.pop();
				}
			}

			if (ptr->m_nodes)
				m_nodes = ptr->m_nodes;
		}
		else
		{
			if (m_tasks.empty())
			{	// No risk.
				RegularScope* ptr = (RegularScope*)m_nodes;
				if (!ptr->m_tasks.empty())
				{
					int line = -1;
					Redirection noredirection = std::make_pair(std::string(""), RedirectionType::LEFT);
					while (!ptr->m_tasks.empty())
					{
						push(ptr->m_tasks.front(), noredirection, nullptr, line);
						ptr->m_tasks.pop();
					}
				}

				if (ptr->m_nodes)
					m_nodes = ptr->m_nodes;
			}
			else
			{

			}
		}
	}
}