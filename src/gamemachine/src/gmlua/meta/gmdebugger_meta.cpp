#include "stdafx.h"
#include "gmdebugger_meta.h"
#include <gamemachine.h>
#include <gmlua.h>

#define NAME "GMDebugger"

BEGIN_NS

namespace luaapi
{
	// {{BEGIN META FUNCTION}}
	GM_LUA_FUNC(info)
	{
		GMLuaArguments args(L, NAME ".info", { GMMetaMemberType::String });
		GMString arg0 = args.getArgument(0).toString();
		gm_info(arg0);
		return GMReturnValues();
	}

	GM_LUA_FUNC(error)
	{
		GMLuaArguments args(L, NAME ".error", { GMMetaMemberType::String });
		GMString arg0 = args.getArgument(0).toString();
		gm_error(arg0);
		return GMReturnValues();
	}

	GM_LUA_FUNC(warning)
	{
		GMLuaArguments args(L, NAME ".warning", { GMMetaMemberType::String });
		GMString arg0 = args.getArgument(0).toString();
		gm_warning(arg0);
		return GMReturnValues();
	}

	GM_LUA_FUNC(debug)
	{
		GMLuaArguments args(L, NAME ".debug", { GMMetaMemberType::String });
		GMString arg0 = args.getArgument(0).toString();
		gm_debug(arg0);
		return GMReturnValues();
	}
	// {{END META FUNCTION}}

	namespace
	{
		GMLuaReg g_meta[] = {
			// {{BEGIN META DECLARATIONS}}
			GM_LUA_DECLARATIONS(info),
			GM_LUA_DECLARATIONS(error),
			GM_LUA_DECLARATIONS(warning),
			GM_LUA_DECLARATIONS(debug),
			// {{END META DECLARATIONS}}
			{ 0, 0 }
		};
	}

	GM_LUA_REGISTER_IMPL(GMDebugger_Meta, NAME, g_meta);
}

END_NS