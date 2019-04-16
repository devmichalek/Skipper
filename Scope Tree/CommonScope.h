#pragma once
#include "cmd.h"
#include <queue>
#include <mutex>
#include <thread>
#include <fstream>

class CommonScope
{
public:
	using m_rofile_type = std::pair<std::fstream*, std::string>;
protected:
	// Task manipulation.
	std::queue<Command*> m_tasks;

	// Output manipulation.
	inline static std::mutex m_omutex;

	// Redirected output manipulation.
	inline static std::vector<m_rofile_type> m_rofiles;
	inline static std::vector<std::mutex> m_romutexes;

public:
	enum M_TYPE
	{
		REGULAR = 0,
		CONCURRENT
	};
	M_TYPE m_type;
	size_t m_children;

	explicit CommonScope(const M_TYPE &newType);
	~CommonScope() {}

	virtual bool addTask(Command*&, std::string&, const char*, int&) = 0;
	virtual bool addScope(CommonScope*, M_TYPE, const char*, int&) = 0;
	virtual bool execute() = 0;
	virtual void destroy() = 0;
	virtual void consolidate() = 0;
	virtual CommonScope* getNextNode();

	static std::vector<m_rofile_type>& rofiles() {
		return m_rofiles;
	}

protected:
	static void print(std::string&&, int&&);
	static void redirect(std::string&&, int&&);
	bool push(Command*&, std::string&, const char*, int&);
	void pop();
};