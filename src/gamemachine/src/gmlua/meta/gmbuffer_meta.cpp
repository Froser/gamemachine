#include "stdafx.h"
#include "gmbuffer_meta.h"
#include <gamemachine.h>
#include <gmlua.h>

using namespace gm::luaapi;

#define NAME "GMBuffer"

GM_LUA_PROXY_GC_IMPL(GMBufferProxy, NAME ".__gc");

bool GMBufferProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META_FUNCTION(__gc);
	return true;
}
