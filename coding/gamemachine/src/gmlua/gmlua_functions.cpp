#include "stdafx.h"
#include "gmlua_functions.h"
#include "foundation/defines.h"
#include "foundation/gamemachine.h"

extern "C"
{
	Map<const char*, lua_CFunction> g_gmlua_functions = {
		{ "gmOutputDebug", gmlua_outputDebug },
		{ "gmDebug", gmlua_debug },
		{ "gmWarning", gmlua_warning },
		{ "gmInfo", gmlua_info },
		{ "gmError", gmlua_error },
	};

	static const char* getString(lua_State* L, const char* caller)
	{
		if (!lua_isstring(L, -1))
			gm_error("[LUA] %s: argument is not a string.", caller);
		return luaL_optstring(L, -1, "");
	}

	int gmlua_outputDebug(lua_State* L)
	{
		const char* arg = getString(L, "gmlua_outputDebug");
#if _WINDOWS
		OutputDebugStringA(arg);
#else
		printf("%s", arg.c_str());
#endif
		lua_pop(L, 1);
		return 0;
	}

	int gmlua_debug(lua_State* L)
	{
		const char* arg = getString(L, "gmOutputDebug");
		gm_debug(arg);
		return 0;
	}

	int gmlua_warning(lua_State* L)
	{
		const char* arg = getString(L, "gmlua_warning");
		gm_warning(arg);
		return 0;
	}

	int gmlua_info(lua_State* L)
	{
		const char* arg = getString(L, "gmlua_info");
		gm_info(arg);
		return 0;
	}

	int gmlua_error(lua_State* L)
	{
		const char* arg = getString(L, "gmlua_error");
		gm_error(arg);
		return 0;
	}
}