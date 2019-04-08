#include "CommonScope.h"

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
	std::lock_guard<std::mutex> locker(m_romutexes[index]);
	if (m_rofiles[index].first->is_open())
		*m_rofiles[index].first << msg;
}

void CommonScope::push(Command* &cmd, std::string &pathToFile)
{
	int output_index = -1;
	if (!pathToFile.empty())
	{	// open next file...
		auto it = m_rofiles.begin();
		for (; it != m_rofiles.end(); ++it)
			if (it->second == pathToFile)
				break;

		if (it == m_rofiles.end())
		{
			std::fstream* file = new std::fstream;
			file->open(pathToFile, std::ios::app);
			output_index = (int)m_rofiles.size();
			m_rofiles.push_back(std::make_pair(file, pathToFile));
		}
		else
			output_index = (int)std::distance(m_rofiles.begin(), it);
	}

	// add next task...
	void(*fun)(std::string &&, int &&) = output_index < 0 ? &print : &redirect;
	m_tasks.push(std::make_pair(cmd, (void*)fun));
}

void CommonScope::pop()
{
	delete m_tasks.front().first;
	m_tasks.pop();
}