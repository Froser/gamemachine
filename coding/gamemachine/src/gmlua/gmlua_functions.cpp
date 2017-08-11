#include "stdafx.h"
#include "gmlua_functions.h"
#include "foundation/defines.h"
#include "foundation/gamemachine.h"

extern "C"
{
	static luaL_Reg g_gmlua_core_functions[] = {
		{ "outputDebug", gmlua_core_outputDebug },
		{ "debug", gmlua_core_debug },
		{ "warning", gmlua_core_warning },
		{ "info", gmlua_core_info },
		{ "error", gmlua_core_error },
		{ 0, 0 }
	};

	int register_core(lua_State *L)
	{
		luaL_newlib(L, g_gmlua_core_functions);
		return 1;
	}

	static const char* getString(lua_State* L, const char* caller)
	{
		if (!lua_isstring(L, -1))
			gm_error("[LUA] %s: argument is not a string.", caller);
		return luaL_optstring(L, -1, "");
	}

	int gmlua_core_outputDebug(lua_State* L)
	{
		const char* arg = getString(L, "outputDebug");
#if _WINDOWS
		OutputDebugStringA(arg);
#else
		printf("%s", arg.c_str());
#endif
		lua_pop(L, 1);
		return 0;
	}

	int gmlua_core_debug(lua_State* L)
	{
		const char* arg = getString(L, "debug");
		gm_debug(arg);
		return 0;
	}

	int gmlua_core_warning(lua_State* L)
	{
		const char* arg = getString(L, "warning");
		gm_warning(arg);
		return 0;
	}

	int gmlua_core_info(lua_State* L)
	{
		const char* arg = getString(L, "info");
		gm_info(arg);
		return 0;
	}

	int gmlua_core_error(lua_State* L)
	{
		const char* arg = getString(L, "error");
		gm_error(arg);
		return 0;
	}
}