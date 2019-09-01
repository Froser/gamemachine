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
		GM_LUA_FUNC(loadShaderOpenGL)
		{
			static const GMString s_invoker = NAME ".loadShaderOpenGL";
			GM_LUA_CHECK_ARG_COUNT(L, 9, NAME ".loadShaderOpenGL");
			IRenderContextProxy context(L);
			GMString manifestPath = GMArgumentHelper::popArgumentAsString(L, s_invoker);
			GMArgumentHelper::popArgumentAsObject(L, context, s_invoker);

			GMBuffer buffer;
			GM.getGamePackageManager()->readFile(GMPackageIndex::Shaders, manifestPath, &buffer, nullptr);
			buffer.convertToStringBuffer();
			GMGLHelper::loadShader(context.get(), GMString((const char*)buffer.getData()));

			return gm::GMReturnValues();
		}

		GM_LUA_FUNC(loadShaderDx11)
		{
			static const GMString s_invoker = NAME ".loadShaderDx11";
			GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".loadShaderDx11");
			IRenderContextProxy context(L);
			GMString path = GMArgumentHelper::popArgumentAsString(L, s_invoker);
			GMArgumentHelper::popArgumentAsObject(L, context, s_invoker);
#if GM_USE_DX11
			GMDx11Helper::loadShader(context.get(), path, L"main.gfx");
#else
			DirectX11LoadShader(context.get(), path);
#endif
			return gm::GMReturnValues();
		}

		GM_LUA_FUNC(loadShader)
		{
			static const GMString s_invoker = NAME ".loadShader";
			GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".loadShader");
			IRenderContextProxy context(L);
			GMArgumentHelper::popArgumentAsObject(L, context, s_invoker);
			GMShaderHelper::loadShader(context.get());
			return gm::GMReturnValues();
		}

		// {{END META FUNCTION}}

		GMLuaReg g_meta[] = {
			// {{BEGIN META DECLARATIONS}}
			GM_LUA_DECLARATIONS(loadShaderOpenGL),
			GM_LUA_DECLARATIONS(loadShaderDx11),
			GM_LUA_DECLARATIONS(loadShader),
			// {{END META DECLARATIONS}}
			{ 0, 0 }
		};
	}

	GM_LUA_REGISTER_IMPL(GMShaderHelper_Meta, NAME, g_meta);
}

END_NS