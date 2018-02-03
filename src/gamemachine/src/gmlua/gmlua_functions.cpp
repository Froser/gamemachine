#include "stdafx.h"
#include "gmlua_functions.h"
#include "foundation/defines.h"
#include "foundation/gamemachine.h"

using namespace luaapi;

extern "C"
{
	static luaL_Reg g_functions[] = {
		{ "gmcore", register_core },
		{ "gmmath", register_math },
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
		{ "output", core_output },
		{ "debug", core_debug },
		{ "warning", core_warning },
		{ "info", core_info },
		{ "error", core_error },
		{ 0, 0 }
	};

	// core
	GM_LUA_API int register_core(lua_State *L)
	{
		luaL_newlib(L, g_core_functions);
		return 1;
	}

	static const char* getString(lua_State* L, const char* caller)
	{
		if (!lua_isstring(L, -1))
			gm_error("[LUA] %s: argument is not a string.", caller);
		return luaL_optstring(L, -1, "");
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

	// math
	static luaL_Reg g_math_functions[] = {
		{ "translate", math_translate },
		{ "scale", math_scale },
		{ "rotate", math_rotate },
		{ "mxm", math_mxm },
		{ "vxm", math_vxm },
		{ "vlerp", math_vlerp },
		{ 0, 0 }
	};

	GM_LUA_API int register_math(lua_State *L)
	{
		luaL_newlib(L, g_math_functions);
		return 1;
	}

	GM_LUA_API int math_translate(lua_State* L)
	{
		GMfloat x = lua_tonumber(L, 1),
			y = lua_tonumber(L, 2),
			z = lua_tonumber(L, 3);
		GMMat4 mat = Translate({ x, y, z });
		GMLua(L).setMatrix(mat);
		return 1;
	}

	GM_LUA_API int math_scale(lua_State* L)
	{
		GMfloat x = lua_tonumber(L, 1),
			y = lua_tonumber(L, 2),
			z = lua_tonumber(L, 3);
		GMMat4 mat = Scale(MakeVector3(x, y, z));
		GMLua(L).setMatrix(mat);
		return 1;
	}

	GM_LUA_API int math_rotate(lua_State* L)
	{
		GMfloat x = lua_tonumber(L, 1),
			y = lua_tonumber(L, 2),
			z = lua_tonumber(L, 3),
			r = lua_tonumber(L, 4);
		GMQuat q = Rotate(r, GMVec3(x, y, z));
		GMMat4 mat = QuatToMatrix(q);
		GMLua(L).setMatrix(mat);
		return 1;
	}

	GM_LUA_API int math_mxm(lua_State* L)
	{
		GMLua lua(L);
		GMMat4 mat1, mat2;
		lua.getMatrix(mat1, 1);
		lua.getMatrix(mat2, 2);
//#warning todo
		//lua.setMatrix(mat1 * mat2);
		return 1;
	}

	GM_LUA_API int math_vxm(lua_State* L)
	{
		GMLua lua(L);
		GMVec4 v;
		GMMat4 m;
		lua.getVector(v, 1);
		lua.getMatrix(m, 2);
		lua.setVector(v * m);
		return 1;
	}

	GM_LUA_API int math_vlerp(lua_State* L)
	{
		GMLua lua(L);
		GMVec4 v1, v2;
		lua.getVector(v1, 1);
		lua.getVector(v2, 2);
		GMfloat p = lua_tonumber(L, 3);
		GMVec4 result = Lerp(v1, v2, p);
		lua.setVector(result);
		return 1;
	}
}