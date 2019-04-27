#pragma once
#include "CommonScope.h"

class ConcurrentScope;

class RegularScope final : public CommonScope
{
	CommonScope* m_upNode;	// parent is either regular or concurrent scope
	CommonScope* m_nodes;	// contains (if there are any) regular or concurrent scopes (only one type)
	RegularScope* m_next;
	friend class ConcurrentScope;
public:
	explicit RegularScope(CommonScope*);
	~RegularScope();

	bool addTask(Command*&, Redirection&, const char*, int&);
	bool addScope(CommonScope*, M_TYPE, const char*, int&);
	bool execute();
	void destroy();
	bool capture(RegularScope*&, const char*, int&); // capture tasks and children from other branch
	RegularScope* getNextNode();
	void consolidate();
};