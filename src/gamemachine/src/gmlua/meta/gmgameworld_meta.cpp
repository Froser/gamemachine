#include "stdafx.h"
#include "gmgameworld_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "irendercontext_meta.h"

using namespace gm::luaapi;

#define NAME "GMGameWorld"

GMGameWorldProxy::GMGameWorldProxy(GMGameWorld* gameworld /*= nullptr*/)
{
	D(d);
	d->gameworld = gameworld;
}

bool GMGameWorldProxy::registerMeta()
{
	GM_META(gameworld);
	GM_META_FUNCTION(renderScene);
	return true;
}

/*
 * __gc([self])
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(GMGameWorldProxy, __gc, L)
{
	static const GMString s_invoker = NAME ".__gc";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".__gc");
	GMGameWorldProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self.release();
	return GMReturnValues();
}

/*
 * renderScene([self])
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(GMGameWorldProxy, renderScene, L)
{
	static const GMString s_invoker = NAME ".renderScene";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".renderScene");
	GMGameWorldProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->renderScene();
	return GMReturnValues();
}

//////////////////////////////////////////////////////////////////////////
namespace
{
	// {{BEGIN META FUNCTION}}
	GM_LUA_META_FUNCTION_IMPL(New, L)
	{
		static const GMString s_invoker = NAME ".new";
		GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".new");
		IRenderContextProxy context;
		GMArgumentHelper::popArgumentAsObject(L, context, s_invoker);
		GMGameWorldProxy proxy(new GMGameWorld(context.get()));
		return GMReturnValues(L, GMVariant(proxy));
	}
	// {{END META FUNCTION}}

	GMLuaReg g_meta[] = {
		// {{BEGIN META DECLARATIONS}}
		{ "new", New },
		// {{END META DECLARATIONS}}
		{ 0, 0 }
	};
}

const char* GMGameWorld_Meta::Name = NAME;

void GMGameWorld_Meta::registerFunctions(GMLua* L)
{
	setRegisterFunction(L, Name, regCallback, true);
}

int GMGameWorld_Meta::regCallback(GMLuaCoreState *L)
{
	newLibrary(L, g_meta);
	return 1;
}