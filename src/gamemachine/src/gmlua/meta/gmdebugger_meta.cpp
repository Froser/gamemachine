#include "stdafx.h"
#include "gmdebugger_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "../gmlua_functions.h"

using namespace gm::luaapi;

#define NAME "GMDebugger"

namespace
{
	// {{BEGIN META FUNCTION}}
	LUA_API int info(GMLuaCoreState* L)
	{
		GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".info");
		const char* arg0 = GMArgumentHelper::getArgumentAsString(L, "info");
		GMDebugger::instance().info(arg0);
		return 0;
	}

	LUA_API int error(GMLuaCoreState* L)
	{
		GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".error");
		const char* arg0 = GMArgumentHelper::getArgumentAsString(L, "error");
		GMDebugger::instance().error(arg0);
		return 0;
	}

	LUA_API int warning(GMLuaCoreState* L)
	{
		GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".warning");
		const char* arg0 = GMArgumentHelper::getArgumentAsString(L, "warning");
		GMDebugger::instance().warning(arg0);
		return 0;
	}

	LUA_API int debug(GMLuaCoreState* L)
	{
		GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".debug");
		const char* arg0 = GMArgumentHelper::getArgumentAsString(L, "debug");
		GMDebugger::instance().debug(arg0);
		return 0;
	}
	// {{END META FUNCTION}}

	luaL_Reg g_meta[] = {
		// {{BEGIN META DECLARATIONS}}
		GM_LUA_DECLARATIONS(info),
		GM_LUA_DECLARATIONS(error),
		GM_LUA_DECLARATIONS(warning),
		GM_LUA_DECLARATIONS(debug),
		// {{END META DECLARATIONS}}
		{ 0, 0 }
	};
}

const char* GMDebugger_Meta::Name = NAME;

void GMDebugger_Meta::registerFunctions(GMLuaCoreState* L)
{
	setRegisterFunction(L, Name, regCallback, true);
}

int GMDebugger_Meta::regCallback(GMLuaCoreState *L)
{
	newLibrary(L, g_meta);
	return 1;
}