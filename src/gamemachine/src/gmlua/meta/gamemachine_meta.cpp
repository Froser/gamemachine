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
	int exit(lua_State* L)
	{
		GM.exit();
		return 0;
	}

	int getGameMachineRunningStates(lua_State* L)
	{
		GMGameMachineRunningStatesProxy r;
		GMLua(L).setTable(r);
		return 1;
	}
	// {{END META FUNCTION}}

	luaL_Reg g_meta[] = {
		// {{BEGIN META DECLARATIONS}}
		GM_LUA_DECLARATIONS(exit),
		GM_LUA_DECLARATIONS(getGameMachineRunningStates),
		// {{END META DECLARATIONS}}
		{ 0, 0 }
	};
}

const char* GameMachine_Meta::Name = "GM";

void GameMachine_Meta::registerFunctions(lua_State* L)
{
	luaL_requiref(L, Name, registerFunction, 1);
	lua_pop(L, 1);
}

int GameMachine_Meta::registerFunction(lua_State *L)
{
	luaL_newlib(L, g_meta);
	return 1;
}