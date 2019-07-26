#ifndef __GM_LUA_RUNTIME_H__
#define __GM_LUA_RUNTIME_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_CLASS(GMLuaRuntime);
class GMLuaRuntime
{
	GM_DECLARE_PRIVATE(GMLuaRuntime)
	GM_DISABLE_COPY_ASSIGN(GMLuaRuntime)

public:
	GMLuaRuntime();
	~GMLuaRuntime();

public:
	bool addObject(IDestroyObject* object);
	bool detachObject(IDestroyObject* object);
	bool containsObject(IDestroyObject* object);
};

END_NS
#endif