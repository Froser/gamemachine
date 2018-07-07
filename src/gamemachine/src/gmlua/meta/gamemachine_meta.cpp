#include "stdafx.h"
#include "gamemachine_meta.h"
#include <gamemachine.h>
#include <gmlua.h>

using namespace gm::luaapi;

namespace
{
	class GMGameMachineRunningStatesProxy : public GMObject
	{
		GM_DECLARE_PRIVATE_FROM_STRUCT(GMGameMachineRunningStatesProxy, GMGameMachineRunningStates)

	public:
		GMGameMachineRunningStatesProxy()
		{
			D(d);
			*d = GM.getGameMachineRunningStates();
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
			GM_META_WITH_TYPE(endiannessMode, GMMetaMemberType::Int);
			return true;
		}
	};

	// {{BEGIN META FUNCTION}}
	LUA_API int exit(GMLuaCoreState* L)
	{
		GM.exit();
		return 0;
	}

	LUA_API int getGameMachineRunningStates(GMLuaCoreState* L)
	{
		GMGameMachineRunningStatesProxy r;
		GMLua(L).setTable(r);
		return 1;
	}
	// {{END META FUNCTION}}

	GMLuaReg g_meta[] = {
		// {{BEGIN META DECLARATIONS}}
		GM_LUA_DECLARATIONS(exit),
		GM_LUA_DECLARATIONS(getGameMachineRunningStates),
		// {{END META DECLARATIONS}}
		{ 0, 0 }
	};
}

const char* GameMachine_Meta::Name = "GM";

void GameMachine_Meta::registerFunctions(GMLuaCoreState* L)
{
	setRegisterFunction(L, Name, regCallback, true);
}

int GameMachine_Meta::regCallback(GMLuaCoreState *L)
{
	newLibrary(L, g_meta);
	return 1;
}