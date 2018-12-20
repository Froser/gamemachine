#include "stdafx.h"
#include "template_meta.h"
#include <gamemachine.h>
#include <gmlua.h>

#define NAME "Template"

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

const char* GMLuaTemplate_Meta::Name = NAME;

void GMLuaTemplate_Meta::registerFunctions(GMLua* L)
{
	setRegisterFunction(L, Name, regCallback, true);
}

int GMLuaTemplate_Meta::regCallback(GMLuaCoreState *L)
{
	newLibrary(L, g_meta);
	return 1;
}