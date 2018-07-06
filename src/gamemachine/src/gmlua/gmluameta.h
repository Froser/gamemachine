#ifndef __GMLUAMETA_H__
#define __GMLUAMETA_H__

// {{BEGIN META INCLUDES}}
#include "meta/gamemachine_meta.h"
// {{END META INCLUDES}}

BEGIN_NS
namespace luaapi
{
	void registerLib(lua_State* L)
	{
		// {{BEGIN META REGISTER}}
		GameMachine_Meta().registerFunctions(L);
		// {{END META REGISTER}}
	}
}
END_NS

#endif