#include "collector.h"
#include <thread>

Collector::Collector()
{

}

Collector::~Collector()
{

}

void Collector::print(std::string &&msg, int &&)
{
	std::lock_guard<std::mutex> locker(m_omutex);
	printf("%s", msg.c_str());
}

void Collector::redirect(std::string &&msg, int &&index)
{
	std::lock_guard<std::mutex> locker(m_romutexes[index]);
	if (m_rofiles[index].first->is_open())
		*m_rofiles[index].first << msg;
}

void Collector::add(Command* cmd, std::string &&pathToFile)
{
	std::unique_lock<std::mutex> locker(m_smutex, std::defer_lock);

	unsigned output_index = -1;
	{	// open next file...
		m_smutex.lock();

		if (!pathToFile.empty())
		{
			auto it = std::find(m_rofiles.begin(), m_rofiles.end(), [&](m_rofile_type &it) {
				return it.second == pathToFile;
			});
			if (it == m_rofiles.end())
			{
				std::fstream* file = new std::fstream;
				file->open(pathToFile, std::ios::app);
				m_rofiles.push_back(std::make_pair(file, pathToFile));
				output_index = m_rofiles.size();
			}
			else
				output_index = std::distance(m_rofiles.begin(), it);
		}

		m_smutex.unlock();
	}

	{	// add next task...
		m_smutex.lock();
		void (*fun)(std::string &&, int &&) = output_index < 0 ? &print : &redirect;
		m_tasks.push_back(std::make_pair(cmd, (void*)fun));
		std::thread thread([&]() {
			Command* ptr = m_tasks.back().first;
			auto it = m_tasks.begin() + (m_tasks.size() - 1);
			ptr->force(fun);
			if (ptr->parse())
				ptr->run();
			remove(it);
		});

		thread.detach();

		m_smutex.unlock();
	}
}

void Collector::remove(std::vector<m_task_type>::iterator &it)
{
	std::lock_guard<std::mutex> locker(m_smutex);
	delete it->first;
	m_tasks.erase(it);
}