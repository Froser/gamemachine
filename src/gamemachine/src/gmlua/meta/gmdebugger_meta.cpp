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
	GM_LUA_FUNC(info, L)
	{
		static const GMString s_invoker = NAME ".info";
		GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".info");
		const char* arg0 = GMArgumentHelper::popArgumentAsString(L, s_invoker);
		gm_info(arg0);
		return GMReturnValues();
	}

	GM_LUA_FUNC(error, L)
	{
		static const GMString s_invoker = NAME ".error";
		GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".error");
		const char* arg0 = GMArgumentHelper::popArgumentAsString(L, s_invoker);
		gm_error(arg0);
		return GMReturnValues();
	}

	GM_LUA_FUNC(warning, L)
	{
		static const GMString s_invoker = NAME ".warning";
		GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".warning");
		const char* arg0 = GMArgumentHelper::popArgumentAsString(L, s_invoker);
		gm_warning(arg0);
		return GMReturnValues();
	}

	GM_LUA_FUNC(debug, L)
	{
		static const GMString s_invoker = NAME ".debug";
		GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".debug");
		const char* arg0 = GMArgumentHelper::popArgumentAsString(L, s_invoker);
		gm_debug(arg0);
		return GMReturnValues();
	}
	// {{END META FUNCTION}}

	GMLuaReg g_meta[] = {
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

void GMDebugger_Meta::registerFunctions(GMLua* L)
{
	setRegisterFunction(L, Name, regCallback, true);
}

int GMDebugger_Meta::regCallback(GMLuaCoreState *L)
{
	newLibrary(L, g_meta);
	return 1;
}