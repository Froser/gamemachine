#include "stdafx.h"
#include "gmgameobject_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "gmasset_meta.h"

#define NAME "GMGameObject"

using namespace gm::luaapi;

GMGameObjectProxy::GMGameObjectProxy(GMGameObject* gameObj /*= nullptr*/)
{
	D(d);
	d->gameObj = gameObj;
}

bool GMGameObjectProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META(gameObj);
	GM_META_FUNCTION(setAsset);
	return true;
}

/*
 * __gc([self])
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(GMGameObjectProxy, __gc, L)
{
	static const GMString s_invoker = NAME ".__gc";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".__gc");
	GMGameObjectProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	self.release();
	return GMReturnValues();
}

/*
 * setAsset([self], GMAsset)
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(GMGameObjectProxy, setAsset, L)
{
	static const GMString s_invoker = NAME ".setAsset";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".setAsset");
	GMGameObjectProxy self;
	GMAssetProxy asset;
	GMArgumentHelper::popArgumentAsObject(L, asset, s_invoker); //asset
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->setAsset(*asset.get());
	return GMReturnValues();
}
namespace
{
	// {{BEGIN META FUNCTION}}
	GM_LUA_META_FUNCTION_IMPL(New, L)
	{
		static const GMString s_invoker = NAME ".new";
		GM_LUA_CHECK_ARG_COUNT(L, 0, NAME ".new");
		GMGameObjectProxy proxy(new GMGameObject());
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

const char* GMGameObject_Meta::Name = NAME;

void GMGameObject_Meta::registerFunctions(GMLua* L)
{
	setRegisterFunction(L, Name, regCallback, true);
}

int GMGameObject_Meta::regCallback(GMLuaCoreState *L)
{
	newLibrary(L, g_meta);
	return 1;
}