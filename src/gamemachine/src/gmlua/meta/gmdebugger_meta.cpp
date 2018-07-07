#include "stdafx.h"
#include "gmdebugger_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "../gmlua_functions.h"

using namespace gm::luaapi;

namespace
{
	// {{BEGIN META FUNCTION}}
	LUA_API int info(GMLuaCoreState* L)
	{
		const char* arg0 = GMArgumentHelper::getArgumentAsString(L, "info");
		GMDebugger::instance().info(arg0);
		return 0;
	}
	// {{END META FUNCTION}}

	luaL_Reg g_meta[] = {
		// {{BEGIN META DECLARATIONS}}
		GM_LUA_DECLARATIONS(info),
		// {{END META DECLARATIONS}}
		{ 0, 0 }
	};
}

const char* GMDebugger_Meta::Name = "GMDebugger";

void GMDebugger_Meta::registerFunctions(GMLuaCoreState* L)
{
	setRegisterFunction(L, Name, regCallback, true);
}

int GMDebugger_Meta::regCallback(GMLuaCoreState *L)
{
	newLibrary(L, g_meta);
	return 1;
}