#include "stdafx.h"
#include "gamemachine_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "../gmlua_functions.h"
#include "iwindow_meta.h"
#include "gmgamepackage_meta.h"

using namespace gm::luaapi;

#define NAME "GM"

namespace
{
	class GMGameMachineRunningStatesProxy : public GMObject
	{
		GM_DECLARE_PRIVATE_FROM_STRUCT(GMGameMachineRunningStatesProxy, GMGameMachineRunningStates)

	public:
		GMGameMachineRunningStatesProxy()
		{
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
			GM_META(lastFrameElpased);
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
		GM_LUA_CHECK_ARG_COUNT(L, 0, NAME ".exit");
		GM.exit();
		return GMReturnValues();
	}

	GM_LUA_FUNC(getRunningStates)
	{
		GMGameMachineRunningStatesProxy proxy;
		GM_LUA_CHECK_ARG_COUNT(L, 0, NAME ".getRunningStates");
		return GMReturnValues (L, GMVariant(proxy));
	}

	GM_LUA_FUNC(getGamePackageManager)
	{
		GMGamePackageProxy proxy(GM.getGamePackageManager());
		GM_LUA_CHECK_ARG_COUNT(L, 0, NAME ".getGamePackageManager");
		return GMReturnValues(L, GMVariant(proxy));
	}

	GM_LUA_FUNC(addWindow)
	{
		IWindowProxy window;
		static const GMString s_invoker = NAME ".addWindow";
		GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".addWindow");
		GMArgumentHelper::beginArgumentReference(L, window, s_invoker);
		window.detach();
		GMArgumentHelper::endArgumentReference(L, window);

		GM.addWindow(window.get());
		return GMReturnValues();
	}

	GM_LUA_FUNC(startGameMachine)
	{
		GM_LUA_CHECK_ARG_COUNT(L, 0, NAME ".startGameMachine");
		GM.startGameMachine();
		return GMReturnValues();
	}

	// {{END META FUNCTION}}

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

const char* GameMachine_Meta::Name = NAME;

void GameMachine_Meta::registerFunctions(GMLua* L)
{
	setRegisterFunction(L, Name, regCallback, true);
}

int GameMachine_Meta::regCallback(GMLuaCoreState *L)
{
	newLibrary(L, g_meta);
	return 1;
}