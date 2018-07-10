#include "stdafx.h"
#include "gamemachine_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "../gmlua_functions.h"

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
		GM_LUA_CHECK_ARG_COUNT(L, 0, NAME ".exit");
		GM.exit();
		return 0;
	}

	LUA_API int getRunningStates(GMLuaCoreState* L)
	{
		GM_LUA_CHECK_ARG_COUNT(L, 0, NAME ".getRunningStates");
		return GMReturnValues (L, GMVariant(GMGameMachineRunningStatesProxy()));
	}
	// {{END META FUNCTION}}

	GMLuaReg g_meta[] = {
		// {{BEGIN META DECLARATIONS}}
		GM_LUA_DECLARATIONS(exit),
		GM_LUA_DECLARATIONS(getRunningStates),
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