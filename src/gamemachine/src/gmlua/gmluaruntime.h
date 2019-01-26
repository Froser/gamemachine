#ifndef __GM_LUA_RUNTIME_H__
#define __GM_LUA_RUNTIME_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMLuaRuntime)
{
	Set<IDestroyObject*> autoReleasePool;
};

class GMLuaRuntime
{
	GM_DECLARE_PRIVATE_NGO(GMLuaRuntime)

public:
	~GMLuaRuntime();

public:
	bool addObject(IDestroyObject* object);
	bool detachObject(IDestroyObject* object);
	bool containsObject(IDestroyObject* object);
};

END_NS
#endif