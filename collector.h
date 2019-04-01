#pragma once
#include "cmd.h"
#include <mutex>
#include <fstream>

class Collector
{
	// stack manip
	typedef std::pair<Command*, void*> m_task_type;
	std::vector<m_task_type> m_tasks;
	std::mutex m_smutex;

	// output manip
	inline static std::mutex m_omutex;

	// redirected output manip
	using m_rofile_type = std::pair<std::fstream*, std::string>;
	inline static std::vector<m_rofile_type> m_rofiles;
	inline static std::vector<std::mutex> m_romutexes;

public:
	explicit Collector();
	Collector(Collector bitand) = delete;
	Collector(Collector &&) = delete;
	compl Collector();
private:
	static void print(std::string &&, int &&);
	static void redirect(std::string &&, int &&);
public:
	void add(Command*, std::string &&);
private:
	void remove(std::vector<m_task_type>::iterator &it);
};