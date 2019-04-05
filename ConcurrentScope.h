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

	bool addTask(Command*, std::string, int&);
	bool addScope(CommonScope*, M_TYPE, int&);
	bool execute();
	ConcurrentScope* getNextNode();
};