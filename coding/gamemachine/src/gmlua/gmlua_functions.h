#ifndef __GMLUA_FUNCTIONS_H__
#define __GMLUA_FUNCTIONS_H__
#include <gmcommon.h>
#include "gmlua.h"
BEGIN_NS

namespace luaapi
{
	extern "C"
	{
		GM_LUA_API void register_functions(lua_State *L);

		GM_LUA_API int register_core(lua_State *L);
		GM_LUA_API int core_output(lua_State *L);
		GM_LUA_API int core_debug(lua_State *L);
		GM_LUA_API int core_warning(lua_State *L);
		GM_LUA_API int core_info(lua_State *L);
		GM_LUA_API int core_error(lua_State *L);

		GM_LUA_API int register_math(lua_State *L);
		GM_LUA_API int math_translate(lua_State* L);
		GM_LUA_API int math_scale(lua_State* L);
		GM_LUA_API int math_rotate(lua_State* L);
		GM_LUA_API int math_mxm(lua_State* L);
		GM_LUA_API int math_vxm(lua_State* L);
		GM_LUA_API int math_vlerp(lua_State* L);
	}
}

END_NS
#endif