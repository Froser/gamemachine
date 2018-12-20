#include "stdafx.h"
#include "ifactory_meta.h"
#include <gamemachine.h>
#include <gmlua.h>

using namespace gm::luaapi;

#define NAME "GMFactory"

namespace
{
	class GMWindowDescProxy : public GMObject
	{
		GM_DECLARE_PRIVATE_FROM_STRUCT(GMWindowDescProxy, GMWindowDesc)

	protected:
		virtual bool registerMeta() override
		{
			GM_META(createNewWindow);
			GM_META(windowName);
			GM_META_WITH_TYPE(dwStyle, GMMetaMemberType::Int);
			GM_META_WITH_TYPE(dwExStyle, GMMetaMemberType::Int);
			// GM_META(rc);
			GM_META(samples);
			return true;
		}
	};

	// {{BEGIN META FUNCTION}}
	LUA_API GMLuaFunctionReturn createWindow(GMLuaCoreState* L)
	{
		static const GMString s_invoker(L".createWindow");
		static GMWindowDescProxy proxy;
		GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".createWindow");
		bool b = GMArgumentHelper::popArgumentAsObject(L, proxy, s_invoker);
		//TODO
		return GMReturnValues();
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