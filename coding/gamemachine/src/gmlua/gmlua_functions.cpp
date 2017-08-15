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
#if _WINDOWS
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
		GMLua_Matrix4x4 mat = linear_math::translate({ x, y, z });
		GMLua(L).setTable(mat);
		return 1;
	}

	GM_LUA_API int math_scale(lua_State* L)
	{
		GMfloat x = lua_tonumber(L, 1),
			y = lua_tonumber(L, 2),
			z = lua_tonumber(L, 3);
		GMLua_Matrix4x4 mat = linear_math::scale(x, y, z);
		GMLua(L).setTable(mat);
		return 1;
	}

	GM_LUA_API int math_rotate(lua_State* L)
	{
		GMfloat x = lua_tonumber(L, 1),
			y = lua_tonumber(L, 2),
			z = lua_tonumber(L, 3),
			r = lua_tonumber(L, 4);
		linear_math::Quaternion q;
		q.setRotation(linear_math::Vector3(x, y, z), r);
		GMLua_Matrix4x4 mat = q.toMatrix();
		GMLua(L).setTable(mat);
		return 1;
	}

	GM_LUA_API int math_mxm(lua_State* L)
	{
		GMLua lua(L);
		GMLua_Matrix4x4 mat1, mat2;
		lua.getTable(mat1, 1);
		lua.getTable(mat2, 2);
		GMLua_Matrix4x4 mat = (linear_math::Matrix4x4) mat1 * mat2;
		lua.setTable(mat);
		return 1;
	}

	GM_LUA_API int math_vxm(lua_State* L)
	{
		GMLua lua(L);
		GMLua_Vector4 v;
		GMLua_Matrix4x4 m;
		lua.getTable(v, 1);
		lua.getTable(m, 2);
		linear_math::Vector4 result = (linear_math::Vector4)v * m;
		GMLua_Vector4 luaVec(result[0], result[1], result[2], result[3]);
		lua.setTable(luaVec);
		return 1;
	}
}