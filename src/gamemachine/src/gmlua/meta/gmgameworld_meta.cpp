#include "stdafx.h"
#include "gmgameworld_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "irendercontext_meta.h"
#include "gmgameobject_meta.h"

using namespace gm::luaapi;

#define NAME "GMGameWorld"

GMGameWorldProxy::GMGameWorldProxy(GMGameWorld* gameworld /*= nullptr*/)
{
	D(d);
	d->gameworld = gameworld;
}

bool GMGameWorldProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META(gameworld);
	GM_META_FUNCTION(renderScene);
	GM_META_FUNCTION(addObjectAndInit);
	GM_META_FUNCTION(addToRenderList);
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

/*
 * addObjectAndInit([self], GMObject)
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(GMGameWorldProxy, addObjectAndInit, L)
{
	static const GMString s_invoker = NAME ".addObjectAndInit";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".addObjectAndInit");
	GMGameWorldProxy self;
	GMGameObjectProxy gameobject;
	GMArgumentHelper::beginArgumentReference(L, gameobject, s_invoker); //GMObject
	gameobject.detach();
	GMArgumentHelper::endArgumentReference(L, gameobject);
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->addObjectAndInit(gameobject.get());
	return GMReturnValues();
}

/*
 * addToRenderList([self], GMObject)
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(GMGameWorldProxy, addToRenderList, L)
{
	static const GMString s_invoker = NAME ".addToRenderList";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".addToRenderList");
	GMGameWorldProxy self;
	GMGameObjectProxy gameobject;
	GMArgumentHelper::popArgumentAsObject(L, gameobject, s_invoker); //GMObject
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->addToRenderList(gameobject.get());
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