#include "CommonScope.h"
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