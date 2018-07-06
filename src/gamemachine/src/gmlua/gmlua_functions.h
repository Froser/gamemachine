#ifndef __GMLUA_FUNCTIONS_H__
#define __GMLUA_FUNCTIONS_H__
#include <gmcommon.h>
#include "gmlua.h"
BEGIN_NS

namespace luaapi
{
	struct GMArgumentHelper
	{
		static const char* getArgumentAsString(lua_State* L, const char* caller);
	};
}

END_NS
#endif