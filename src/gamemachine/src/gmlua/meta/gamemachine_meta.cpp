#include "stdafx.h"
#include "gamemachine_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "iwindow_meta.h"
#include "gmgamepackage_meta.h"

#define NAME "GM"

BEGIN_NS

namespace luaapi
{
	GM_LUA_PRIVATE_CLASS_FROM(GMGameMachineRunningStatesProxy, GMGameMachineRunningStates)
	class GMGameMachineRunningStatesProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(GMGameMachineRunningStatesProxy)

	public:
		GMGameMachineRunningStatesProxy()
		{
			GM_CREATE_DATA();
			D(d);
			*d = GM.getRunningStates();
		}

		GMGameMachineRunningStatesProxy& operator= (GMGameMachineRunningStatesProxy&& p) GM_NOEXCEPT
		{
			D(d);
			D_OF(dp, &p);
			*d = *dp;
			return *this;
		}

	protected:
		virtual bool registerMeta() override
		{
			GM_META(lastFrameElapsed);
			GM_META(elapsedTime);
			GM_META(fps);
			GM_META(crashDown);
			GM_META_WITH_TYPE(renderEnvironment, GMMetaMemberType::Int);
			GM_META(farZ);
			GM_META(nearZ);
			GM_META_WITH_TYPE(systemInfo.endiannessMode, GMMetaMemberType::Int);
			GM_META_WITH_TYPE(systemInfo.numberOfProcessors, GMMetaMemberType::Int);
			GM_META_WITH_TYPE(systemInfo.processorArchitecture, GMMetaMemberType::Int);
			return true;
		}
	};

	// {{BEGIN META FUNCTION}}
	GM_LUA_FUNC(exit)
	{
		GMLuaArguments(L, NAME ".exit");
		GM.exit();
		return GMReturnValues();
	}

	GM_LUA_FUNC(getRunningStates)
	{
		GMGameMachineRunningStatesProxy proxy;
		GMLuaArguments(L, NAME ".getRunningStates");
		return GMReturnValues (L, GMVariant(proxy));
	}

	GM_LUA_FUNC(getGamePackageManager)
	{
		GMGamePackageProxy proxy(L);
		proxy.set(GM.getGamePackageManager());
		GMLuaArguments(L, NAME ".getGamePackageManager");
		return GMReturnValues(L, GMVariant(proxy));
	}

	GM_LUA_FUNC(addWindow)
	{
		IWindowProxy window(L);
		static const GMString s_invoker = NAME ".addWindow";
		GMLuaArguments args(L, NAME ".addWindow", { gm::GMMetaMemberType::Object });
		args.getArgument(0, &window);
		window.setAutoRelease(false);
		GM.addWindow(window.get());
		return GMReturnValues();
	}

	GM_LUA_FUNC(startGameMachine)
	{
		GM.startGameMachine();
		return GMReturnValues();
	}

	// {{END META FUNCTION}}

	namespace
	{
		GMLuaReg g_meta[] = {
			// {{BEGIN META DECLARATIONS}}
			GM_LUA_DECLARATIONS(exit),
			GM_LUA_DECLARATIONS(getRunningStates),
			GM_LUA_DECLARATIONS(getGamePackageManager),
			GM_LUA_DECLARATIONS(addWindow),
			GM_LUA_DECLARATIONS(startGameMachine),
			// {{END META DECLARATIONS}}
			{ 0, 0 }
		};
	}

	GM_LUA_REGISTER_IMPL(GameMachine_Meta, NAME, g_meta);
}

END_NS