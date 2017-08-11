#ifndef __GMLUA_FUNCTIONS_H__
#define __GMLUA_FUNCTIONS_H__
#include "common.h"
#include "gmlua.h"
BEGIN_NS

extern "C"
{
	extern int gmlua_debugPrint(lua_State *L);
	extern int gmlua_outputDebug(lua_State *L);
	extern int gmlua_debug(lua_State *L);
	extern int gmlua_warning(lua_State *L);
	extern int gmlua_info(lua_State *L);
	extern int gmlua_error(lua_State *L);

	extern Map<const char*, lua_CFunction> g_gmlua_functions;
}

END_NS
#endif