#include "stdafx.h"
#include "gmshaderhelper_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "irendercontext_meta.h"

#if GM_USE_DX11
#include <gmdx11helper.h>
#endif
#include <gmglhelper.h>
#include "wrapper/dx11wrapper.h"
#include "gmengine/gmshaderhelper.h"

#define NAME "GMShaderHelper"

BEGIN_NS

namespace luaapi
{
	namespace
	{
		// {{BEGIN META FUNCTION}}
		GM_LUA_FUNC(loadShader)
		{
			GMLuaArguments args(L, NAME ".loadShader", { GMMetaMemberType::Object });
			IRenderContextProxy context(L);
			args.getArgument(0, &context);
			GMShaderHelper::loadShader(context.get());
			return GMReturnValues();
		}

		// {{END META FUNCTION}}

		GMLuaReg g_meta[] = {
			// {{BEGIN META DECLARATIONS}}
			GM_LUA_DECLARATIONS(loadShader),
			// {{END META DECLARATIONS}}
			{ 0, 0 }
		};
	}

	GM_LUA_REGISTER_IMPL(GMShaderHelper_Meta, NAME, g_meta);
}

END_NS