#include "stdafx.h"
#include "gmlua_functions.h"

BEGIN_NS
using namespace luaapi;

const char* GMArgumentHelper::getArgumentAsString(lua_State* L, const char* caller)
{
	if (!lua_isstring(L, -1))
		gm_error("[LUA] %s: argument is not a string.", caller);
	return luaL_optstring(L, -1, "");
}

END_NS