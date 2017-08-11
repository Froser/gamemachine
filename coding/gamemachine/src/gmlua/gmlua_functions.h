#ifndef __GMLUA_FUNCTIONS_H__
#define __GMLUA_FUNCTIONS_H__
#include "common.h"
#include "gmlua.h"
BEGIN_NS

extern "C"
{
	extern int register_core(lua_State *L);
	extern int gmlua_core_output(lua_State *L);
	extern int gmlua_core_debug(lua_State *L);
	extern int gmlua_core_warning(lua_State *L);
	extern int gmlua_core_info(lua_State *L);
	extern int gmlua_core_error(lua_State *L);
}

END_NS
#endif