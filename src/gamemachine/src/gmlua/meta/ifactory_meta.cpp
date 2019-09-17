#include "stdafx.h"
#include "ifactory_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "iwindow_meta.h"
#include "ilight_meta.h"

BEGIN_NS

namespace luaapi
{

#define NAME "GMFactory"

	namespace
	{
		// {{BEGIN META FUNCTION}}
		GM_LUA_FUNC(createWindow)
		{
			static const GMString s_invoker = NAME ".createWindow";
			GMLuaArguments(L, NAME ".createWindow");
			IWindow* window = nullptr;
			GM.getFactory()->createWindow(0, nullptr, &window);
			return gm::GMReturnValues(L, IWindowProxy(L, window));
		}

		GM_LUA_FUNC(createLight)
		{
			static const GMString s_invoker = NAME ".createLight";
			GMLuaArguments args(L, NAME ".createLight", { GMMetaMemberType::Int });
			ILight* light = nullptr;
			GMLightType type = static_cast<GMLightType>(args.getArgument(0).toInt());
			GM.getFactory()->createLight(type, &light);
			return gm::GMReturnValues(L, ILightProxy(L, light));
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

	GM_LUA_REGISTER_IMPL(IFactory_Meta, NAME, g_meta);
}

END_NS