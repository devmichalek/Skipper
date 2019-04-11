#pragma once
#include "cmd.h"
#include <queue>
#include <mutex>
#include <thread>
#include <fstream>

class CommonScope
{
protected:
	// task manipulation
	typedef std::pair<Command*, void*> m_task_type;
	std::queue<m_task_type> m_tasks;

	// output manipulation
	inline static std::mutex m_omutex;

	// redirected output manipulation
	using m_rofile_type = std::pair<std::fstream*, std::string>;
	inline static std::vector<m_rofile_type> m_rofiles;
	inline static std::vector<std::mutex> m_romutexes;

public:
	enum M_TYPE
	{
		REGULAR = 0,
		CONCURRENT
	};
	M_TYPE m_type;

	explicit CommonScope(const M_TYPE &newType) { m_type = newType; }
	~CommonScope() {}

	virtual bool addTask(Command*&, std::string&, int&) = 0;
	virtual bool addScope(CommonScope*, M_TYPE, int&) = 0;
	virtual bool execute() = 0;
	virtual void destroy() = 0;
	virtual CommonScope* getNextNode();

protected:
	static void print(std::string &&, int &&);
	static void redirect(std::string &&, int &&);
	bool push(Command*&, std::string &, int &);
	void pop();
};