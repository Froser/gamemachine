#include "stdafx.h"
#include "ifactory_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "iwindow_meta.h"

using namespace gm::luaapi;

#define NAME "GMFactory"

namespace
{
	// {{BEGIN META FUNCTION}}
	LUA_API GMLuaFunctionReturn createWindow(GMLuaCoreState* L)
	{
		static const GMString s_invoker(L".createWindow");
		GM_LUA_CHECK_ARG_COUNT(L, 0, NAME ".createWindow");
		IWindow* window = nullptr;
		GM.getFactory()->createWindow(0, nullptr, &window);
		return GMReturnValues(L, GMWindowProxy(window));
	}

	// {{END META FUNCTION}}

	GMLuaReg g_meta[] = {
		// {{BEGIN META DECLARATIONS}}
		GM_LUA_DECLARATIONS(createWindow),
		// {{END META DECLARATIONS}}
		{ 0, 0 }
	};
}

const char* IFactory_Meta::Name = NAME;

void IFactory_Meta::registerFunctions(GMLua* L)
{
	setRegisterFunction(L, Name, regCallback, true);
}

int IFactory_Meta::regCallback(GMLuaCoreState *L)
{
	newLibrary(L, g_meta);
	return 1;
}