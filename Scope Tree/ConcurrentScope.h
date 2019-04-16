#pragma once
#include "CommonScope.h"

class RegularScope;

class ConcurrentScope final : public CommonScope
{
	RegularScope* m_upNode;	// parent is always a regular scope
	RegularScope* m_nodes;	// contains only (if there are any) regular scopes (without exception)
	ConcurrentScope* m_next;
	friend class RegularScope;
public:
	explicit ConcurrentScope(RegularScope*&);
	~ConcurrentScope();

	bool addTask(Command*&, std::string&, const char*, int&);
	bool addScope(CommonScope*, M_TYPE, const char*, int&);
	bool execute();
	void destroy();
	bool capture(RegularScope*&, const char*, int&); // capture tasks and children from other branch
	ConcurrentScope* getNextNode();
	void consolidate();
};