#ifndef __GMLUA_FUNCTIONS_H__
#define __GMLUA_FUNCTIONS_H__
#include <gmcommon.h>
#include "gmlua.h"
BEGIN_NS

#define GM_LUA_CHECK_ARG_COUNT(L, count, invoker) \
	if (gm::luaapi::GMArgumentHelper::getArgumentsCount(L) != count) \
	{ gm_error(L"GMLua : Error occurs while invoking {0}. Reason: Wrong argument count. {1} is expected.", { invoker, gm::GMString(count) }); return 0; }

namespace luaapi
{
	struct GMArgumentHelper
	{
		static GMint getArgumentsCount(GMLuaCoreState* L);
		static const char* getArgumentAsString(GMLuaCoreState* L, const char* invoker);
		static void getArgumentAsObject(GMLuaCoreState* L, REF GMObject& obj, const char* invoker);
	};
}

END_NS
#endif