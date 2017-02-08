#ifndef __SCRIPT_H__
#define __SCRIPT_H__
#include "common.h"
#include <string>
#include <vector>
#include "utilities/autoptr.h"
#include "script_private.h"
BEGIN_NS

class GameWorld;
typedef std::vector<std::string> Arguments;

struct IInvokable
{
	IInvokable() {}
	virtual void invoke(GameWorld* world, Arguments& args) = 0;
};

class Script
{
	DEFINE_PRIVATE(Script);

public:
	Script(GameWorld* world);

public:
	void loadScript(const char* filename);
	void invoke(std::string& func, Arguments& args);
	GameWorld* getWorld() { return m_world; }

private:
	GameWorld* m_world;
};

END_NS
#endif