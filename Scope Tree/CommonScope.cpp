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
	std::lock_guard<std::mutex> locker(m_romutexes[index]);
	if (m_rofiles[index].first->is_open())
		*m_rofiles[index].first << msg;
}

bool CommonScope::push(Command* &cmd, std::string &pathToFile, const char* filename, int &line)
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

			if (!file->is_open())
			{	// File error.
				std::string msg = "Cannot open " + pathToFile;
				PrintError(filename, line, msg.c_str());
				delete cmd;
				cmd = nullptr;
				pathToFile.clear();
				return false;
			}

			output_index = (int)m_rofiles.size();
			m_rofiles.push_back(std::make_pair(file, pathToFile));
		}
		else
			output_index = (int)std::distance(m_rofiles.begin(), it);
	}

	// Add next task.
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