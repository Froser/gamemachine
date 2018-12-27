#include "stdafx.h"
#include "gmshaderhelper_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "irendercontext_meta.h"

#if GM_USE_DX11
#include <gmdx11helper.h>
#endif
#include <gmglhelper.h>

#define NAME "GMShaderHelper"

using namespace gm::luaapi;

namespace
{
	// {{BEGIN META FUNCTION}}
	GM_LUA_META_FUNCTION_IMPL(loadShaderOpenGL, L)
	{
		static const GMString s_invoker = NAME ".loadShaderOpenGL";
		GM_LUA_CHECK_ARG_COUNT(L, 9, NAME ".loadShaderOpenGL");
		IRenderContextProxy context;
		GMString filtersPixelFilePath = GMArgumentHelper::popArgumentAsString(L, s_invoker);
		GMString filtersVertexFilePath = GMArgumentHelper::popArgumentAsString(L, s_invoker);
		GMString deferredLightPixelFilePath = GMArgumentHelper::popArgumentAsString(L, s_invoker);
		GMString deferredLightVertexFilePath = GMArgumentHelper::popArgumentAsString(L, s_invoker);
		GMString deferredGeometryPixelFilePath = GMArgumentHelper::popArgumentAsString(L, s_invoker);
		GMString deferredGeometryVertexFilePath = GMArgumentHelper::popArgumentAsString(L, s_invoker);
		GMString forwardPixelFilePath = GMArgumentHelper::popArgumentAsString(L, s_invoker);
		GMString forwardVertexFilePath = GMArgumentHelper::popArgumentAsString(L, s_invoker);
		GMArgumentHelper::popArgumentAsObject(L, context, s_invoker);

		GMGLHelper::loadShader(
			context.get(),
			forwardVertexFilePath,
			forwardPixelFilePath,
			deferredGeometryVertexFilePath,
			deferredGeometryPixelFilePath,
			deferredLightVertexFilePath,
			deferredLightPixelFilePath,
			filtersVertexFilePath,
			filtersPixelFilePath
		);

		return GMReturnValues();
	}

	GM_LUA_META_FUNCTION_IMPL(loadShaderDx11, L)
	{
#if GM_USE_DX11
		static const GMString s_invoker = NAME ".loadShaderDx11";
		GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".loadShaderDx11");
		IRenderContextProxy context;
		GMString path = GMArgumentHelper::popArgumentAsString(L, s_invoker);
		GMArgumentHelper::popArgumentAsObject(L, context, s_invoker);
		GMDx11Helper::loadShader(context.get(), path);
#else
#endif
		return GMReturnValues();
	}

	// {{END META FUNCTION}}

	GMLuaReg g_meta[] = {
		// {{BEGIN META DECLARATIONS}}
		GM_LUA_DECLARATIONS(loadShaderOpenGL),
		GM_LUA_DECLARATIONS(loadShaderDx11),
		// {{END META DECLARATIONS}}
		{ 0, 0 }
	};
}

const char* GMShaderHelper_Meta::Name = NAME;

void GMShaderHelper_Meta::registerFunctions(GMLua* L)
{
	setRegisterFunction(L, Name, regCallback, true);
}

int GMShaderHelper_Meta::regCallback(GMLuaCoreState *L)
{
	newLibrary(L, g_meta);
	return 1;
}