#include "stdafx.h"
#include "gmlua_functions.h"

BEGIN_NS
using namespace luaapi;

GMint GMArgumentHelper::getArgumentsCount(GMLuaCoreState* L)
{
	return lua_gettop(L);
}

const char* GMArgumentHelper::getArgumentAsString(GMLuaCoreState* L, const char* invoker)
{
	if (!lua_isstring(L, -1))
		gm_error("GMLua (getArgumentAsString): {0}: argument is not a string.", { invoker });
	return luaL_optstring(L, -1, "");
}

void GMArgumentHelper::getArgumentAsObject(GMLuaCoreState* L, REF GMObject& obj, const char* invoker)
{
	if (!GMLua(L).popTable(obj))
		gm_error("GMLua (getArgumentAsObject): {0}: argument is not an object.", { invoker });
}

END_NS