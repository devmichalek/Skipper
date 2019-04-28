#include "CommonScope.h"
#include "RegularScope.h"
#include "ConcurrentScope.h"
#include "Console.h"

CommonScope::CommonScope(const M_TYPE &newType)
{
	m_children = 0;
	m_type = newType;
}

CommonScope* CommonScope::getNextNode()
{
	return nullptr;
}

void CommonScope::print(std::string &&msg, int &&)
{
	std::lock_guard<std::mutex> locker(m_omutex);
	printf("%s", msg.c_str());
}

void CommonScope::redirect(std::string &&msg, int &&index)
{
	std::lock_guard<std::mutex> locker(*m_romutexes[index]);
	std::fstream* fptr = std::get<std::fstream*>(m_rofiles[index]);
	if (fptr->is_open())
		*fptr << msg;
}

bool CommonScope::push(Command* &cmd, Redirection &redirection, const char* filename, int &line)
{
	std::string pathToFile = redirection.first;

	int output_index = -1;
	if (!pathToFile.empty())
	{	// Open the next file.
		auto it = m_rofiles.begin();
		for (; it != m_rofiles.end(); ++it)
			if (std::get<std::string>(*it) == pathToFile)
				break;

		if (it == m_rofiles.end())
		{
			std::fstream* file = new std::fstream;
			std::ios::openmode openmode;
			switch (redirection.second)
			{
				case RedirectionType::RIGHT:
					openmode = std::ios::out | std::ios::trunc;
					break;
				default:
					openmode = std::ios::app;
			}
			file->open(pathToFile, openmode);

			if (!file->is_open())
			{	// File error.
				std::string msg = "Cannot open " + pathToFile;
				PrintError(filename, line, msg.c_str());
				delete cmd;
				cmd = nullptr;
				redirection = std::make_pair(std::string(""), RedirectionType::LEFT);
				return false;
			}

			output_index = (int)m_rofiles.size();
			m_rofiles.push_back(std::make_tuple(file, pathToFile, redirection.second));
			m_romutexes.push_front(std::make_unique<std::mutex>());
		}
		else
		{
			RedirectionType rt = std::get<RedirectionType>(*it);
			if (rt != redirection.second)
			{	// Redirection type error.
				std::string msg = "Cannot change redirection type for " + pathToFile;
				PrintError(filename, line, msg.c_str());
				delete cmd;
				cmd = nullptr;
				redirection = std::make_pair(std::string(""), RedirectionType::LEFT);
				return false;
			}
			output_index = (int)std::distance(m_rofiles.begin(), it);
		}	
	}

	// Add next task.
	pathToFile.clear();
	void(*fun)(std::string &&, int &&) = output_index < 0 ? &print : &redirect;
	cmd->force((void*)fun, output_index);
	m_tasks.push(cmd);
	return true;
}

void CommonScope::pop()
{
	delete m_tasks.front();
	m_tasks.pop();
}

void chip(CommonScope* parent, CommonScope* current)
{
	if (parent->m_children)
	{	// Parent node has children.
		if ((parent->m_type == CommonScope::REGULAR && ((RegularScope*)parent)->m_nodes->m_type == current->m_type) ||
			(parent->m_type == CommonScope::CONCURRENT && ((ConcurrentScope*)parent)->m_nodes->m_type == current->m_type))
		{	// Children of parent node has the same type as current node.
			// Get children of parent ptr.
			CommonScope* ptr = nullptr;
			if (parent->m_type == CommonScope::REGULAR)
				ptr = ((RegularScope*)parent)->m_nodes;
			else
				ptr = ((ConcurrentScope*)parent)->m_nodes;
			CommonScope* nextPtr = ptr->getNextNode();

			// Search for matching ptr based on next ptr.
			while (nextPtr)
			{
				if (nextPtr == current)
					break;
				ptr = ptr->getNextNode();
				nextPtr = nextPtr->getNextNode();
			}

			if (nextPtr)
			{
				CommonScope* todelete = nextPtr;
				if (ptr->m_type == CommonScope::REGULAR)
				{
					((RegularScope*)ptr)->m_next = (RegularScope*)nextPtr->getNextNode();
					((RegularScope*)todelete)->m_nodes = nullptr;
					((RegularScope*)todelete)->m_next = nullptr;
					((RegularScope*)todelete)->m_upNode = nullptr;
				}
				else
				{
					((ConcurrentScope*)ptr)->m_next = (ConcurrentScope*)nextPtr->getNextNode();
					((ConcurrentScope*)todelete)->m_nodes = nullptr;
					((ConcurrentScope*)todelete)->m_next = nullptr;
					((ConcurrentScope*)todelete)->m_upNode = nullptr;
				}

				while (!todelete->m_tasks.empty())
					todelete->m_tasks.pop();

				delete todelete;
				todelete = nullptr;
				--parent->m_children;
			}
			else
			{
				// Ascribe again.
				if (parent->m_type == CommonScope::REGULAR)
					ptr = ((RegularScope*)parent)->m_nodes;
				else
					ptr = ((ConcurrentScope*)parent)->m_nodes;

				if (ptr == current)
				{	// It can be only the first one.
					if (parent->m_type == CommonScope::REGULAR)
						((RegularScope*)parent)->m_nodes = ptr->getNextNode();
					else
						((ConcurrentScope*)parent)->m_nodes = (RegularScope*)ptr->getNextNode();

					CommonScope* todelete = ptr;
					if (ptr->m_type == CommonScope::REGULAR)
					{
						((RegularScope*)todelete)->m_next = nullptr;
						((RegularScope*)todelete)->m_nodes = nullptr;
						((RegularScope*)todelete)->m_upNode = nullptr;
					}
					else
					{
						((ConcurrentScope*)todelete)->m_next = nullptr;
						((ConcurrentScope*)todelete)->m_nodes = nullptr;
						((ConcurrentScope*)todelete)->m_upNode = nullptr;
					}

					while (!todelete->m_tasks.empty())
						todelete->m_tasks.pop();

					delete todelete;
					todelete = nullptr;
					--parent->m_children;
				}
			}
		}
	}
}

void consolidate(CommonScope* parent, CommonScope* current)
{
	// Go to the bottom.
	if (current->m_children)
	{
		CommonScope* ptr = nullptr;
		if (current->m_type == CommonScope::REGULAR)
			ptr = ((RegularScope*)current)->m_nodes;
		else
			ptr = ((ConcurrentScope*)current)->m_nodes;

		while (ptr)
		{
			consolidate(current, ptr);
			ptr = ptr->getNextNode();
		}
	}

	bool update = true;
	while (update)
	{
		update = false;
		if (current->m_children)
		{
			CommonScope* ptr = nullptr;
			if (current->m_type == CommonScope::REGULAR)
				ptr = ((RegularScope*)current)->m_nodes;
			else
				ptr = ((ConcurrentScope*)current)->m_nodes;

			// If there are children continue.
			while (current->m_children && ptr)
			{
				// 1. Current node contains only one concurrent child scope / or
				// 2. There are no grandchildren of current node.
				if (ptr->m_type == CommonScope::CONCURRENT && (current->m_children == 1 ||
					(!((ConcurrentScope*)ptr)->m_nodes) && ptr->m_tasks.empty()))
				{
					ConcurrentScope* child = (ConcurrentScope*)ptr;
					if (!child->m_tasks.empty())
					{
						int line = -1;
						Redirection noredirection = std::make_pair(std::string(""), RedirectionType::LEFT);
						while (!child->m_tasks.empty())
						{
							current->push(child->m_tasks.front(), noredirection, nullptr, line);
							child->m_tasks.pop();
						}
					}

					// Delete and switch.
					CommonScope* switcher = child->getNextNode();
					chip(current, child);
					ptr = switcher;
					update = true;
					break;
				}
				else if (ptr->m_type == CommonScope::REGULAR)
				{
					// 1. There are no tasks inside child scope / or
					// 2. There are tasks inside child scope but no grandchildren / or
					// 3. There are tasks and grandchildren of regular type.
					if (current->m_tasks.empty() || (!current->m_tasks.empty() &&
						(!ptr->m_children || (ptr->m_children && ((RegularScope*)ptr)->m_nodes->m_type == CommonScope::REGULAR))))
					{
						RegularScope* child = (RegularScope*)ptr;

						// Child has tasks. Child tasks are now our tasks.
						if (!child->m_tasks.empty())
						{
							int line = -1;
							Redirection noredirection = std::make_pair(std::string(""), RedirectionType::LEFT);
							while (!child->m_tasks.empty())
							{
								current->push(child->m_tasks.front(), noredirection, nullptr, line);
								child->m_tasks.pop();
							}
						}

						// Child has children. Grandchildren are now our children...
						if (child->m_children)
						{
							if (child->m_nodes->m_type == CommonScope::REGULAR)
							{
								// Get subsub.
								RegularScope* subChildBeginPtr = (RegularScope*)child->m_nodes;
								RegularScope* subChildEndPtr = subChildBeginPtr->m_next;
								if (subChildEndPtr)
								{
									while (subChildEndPtr->m_next)
										subChildEndPtr = subChildEndPtr->m_next;
								}
								else
									subChildEndPtr = subChildBeginPtr;

								{	// Switch parents.
									RegularScope* psPtr = (RegularScope*)child->m_nodes;
									while (psPtr)
									{
										psPtr->m_upNode = current;
										psPtr = psPtr->m_next;
									}
								}

								RegularScope* subPtr = (RegularScope*)((RegularScope*)current)->m_nodes;
								RegularScope* subNextPtr = subPtr->getNextNode();
								RegularScope* todelete = nullptr;
								while (subNextPtr)
								{
									if (subNextPtr == ptr)
										break;
									subPtr = subNextPtr;
									subNextPtr = subNextPtr->getNextNode();
								}
								if (subNextPtr)
								{	// Middle or Last.
									todelete = subNextPtr;
									subPtr->m_next = subChildEndPtr;
									subChildEndPtr->m_next = subNextPtr->m_next;
								}
								else
								{	// First.
									todelete = subPtr;
									subPtr = (RegularScope*)((RegularScope*)current)->m_nodes;
									subChildEndPtr->m_next = subPtr->m_next;
									((RegularScope*)current)->m_nodes = subChildBeginPtr;
								}
								ptr = todelete->m_next;
								todelete->m_nodes = nullptr;
								todelete->m_next = nullptr;
								todelete->m_upNode = nullptr;
								delete todelete;
								todelete = nullptr;
								continue;
							}
							else
							{
								ptr = ptr->getNextNode();
								continue;
							}
						}

						// Delete and switch.
						CommonScope* switcher = child->getNextNode();
						chip(current, child);
						ptr = switcher;
						update = true;
						continue;
					}
				}

				ptr = ptr->getNextNode();
			}
		}
	}
}