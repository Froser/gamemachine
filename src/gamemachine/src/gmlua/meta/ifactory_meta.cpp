#include "stdafx.h"
#include "ifactory_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "iwindow_meta.h"
#include "ilight_meta.h"

using namespace gm::luaapi;

#define NAME "GMFactory"

namespace
{
	// {{BEGIN META FUNCTION}}
	GM_LUA_FUNC(createWindow)
	{
		static const GMString s_invoker = NAME ".createWindow";
		GM_LUA_CHECK_ARG_COUNT(L, 0, NAME ".createWindow");
		IWindow* window = nullptr;
		GM.getFactory()->createWindow(0, nullptr, &window);
		return GMReturnValues(L, IWindowProxy(window));
	}

	GM_LUA_FUNC(createLight)
	{
		static const GMString s_invoker = NAME ".createLight";
		GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".createLight");
		ILight* light = nullptr;
		GMLightType type = static_cast<GMLightType>(GMArgumentHelper::popArgument(L, s_invoker).toInt());
		GM.getFactory()->createLight(type, &light);
		return GMReturnValues(L, ILightProxy(light));
	}
	// {{END META FUNCTION}}

	GMLuaReg g_meta[] = {
		// {{BEGIN META DECLARATIONS}}
		GM_LUA_DECLARATIONS(createWindow),
		GM_LUA_DECLARATIONS(createLight),
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