#include "stdafx.h"
#include "gmlua_functions.h"
#include "foundation/defines.h"
#include "foundation/gamemachine.h"

using namespace luaapi;
BEGIN_NS

extern "C"
{
	static luaL_Reg g_functions[] = {
		{ "gmcore", register_core },
		{ 0, 0 }
	};

	GM_LUA_API void register_functions(lua_State *L)
	{
		const luaL_Reg *lib;
		for (lib = g_functions; lib->func; lib++) {
			luaL_requiref(L, lib->name, lib->func, 1);
			lua_pop(L, 1);
		}
	}

	static luaL_Reg g_core_functions[] = {
		{ "output", luaapi::core_output },
		{ "debug", luaapi::core_debug },
		{ "warning", luaapi::core_warning },
		{ "info", luaapi::core_info },
		{ "error", luaapi::core_error },
		{ 0, 0 }
	};

	static const char* getString(lua_State* L, const char* caller)
	{
		if (!lua_isstring(L, -1))
			gm_error("[LUA] %s: argument is not a string.", caller);
		return luaL_optstring(L, -1, "");
	}

	// core
	GM_LUA_API int register_core(lua_State *L)
	{
		luaL_newlib(L, g_core_functions);
		return 1;
	}

	GM_LUA_API int core_output(lua_State* L)
	{
		const char* arg = getString(L, "outputDebug");
#if GM_WINDOWS
		OutputDebugStringA(arg);
#else
		printf("%s", arg.c_str());
#endif
		return 0;
	}

	GM_LUA_API int core_debug(lua_State* L)
	{
		const char* arg = getString(L, "debug");
		gm_debug(arg);
		return 0;
	}

	GM_LUA_API int core_warning(lua_State* L)
	{
		const char* arg = getString(L, "warning");
		gm_warning(arg);
		return 0;
	}

	GM_LUA_API int core_info(lua_State* L)
	{
		const char* arg = getString(L, "info");
		gm_info(arg);
		return 0;
	}

	GM_LUA_API int core_error(lua_State* L)
	{
		const char* arg = getString(L, "error");
		gm_error(arg);
		return 0;
	}
}

END_NS