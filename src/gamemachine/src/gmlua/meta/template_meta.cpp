#include "stdafx.h"
#include "gmdebugger_meta.h"


#include <gamemachine.h>
#include <gmlua.h>

using namespace gm::luaapi;

namespace
{
	// {{BEGIN META FUNCTION}}
	// {{END META FUNCTION}}

	GMLuaReg g_meta[] = {
		// {{BEGIN META DECLARATIONS}}
		// {{END META DECLARATIONS}}
		{ 0, 0 }
	};
}

const char* GMLuaTemplate_Meta::Name = "GM";

void GMLuaTemplate_Meta::registerFunctions(GMLuaCoreState* L)
{
}

int GMLuaTemplate_Meta::regCallback(GMLuaCoreState *L)
{
	return 1;
}