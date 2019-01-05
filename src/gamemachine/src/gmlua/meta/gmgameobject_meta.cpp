#include "stdafx.h"
#include "gmgameobject_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "gmasset_meta.h"

#define NAME "GMGameObject"

using namespace gm::luaapi;

bool GMGameObjectProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META_FUNCTION(setAsset);
	GM_META_FUNCTION(setTranslation);
	GM_META_FUNCTION(setRotation);
	GM_META_FUNCTION(setScaling);
	return true;
}

/*
 * __gc([self])
 */
GM_LUA_PROXY_IMPL(GMGameObjectProxy, __gc)
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
GM_LUA_PROXY_IMPL(GMGameObjectProxy, setAsset)
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

/*
 * setTranslation([self], matrix)
 */
GM_LUA_PROXY_IMPL(GMGameObjectProxy, setTranslation)
{
	static const GMString s_invoker = NAME ".setTranslation";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".setTranslation");
	GMGameObjectProxy self;
	GMMat4 mat = GMArgumentHelper::popArgumentAsMat4(L, s_invoker).toMat4(); //matrix
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->setTranslation(mat);
	return GMReturnValues();
}

/*
* setTranslation([self], quat)
*/
GM_LUA_PROXY_IMPL(GMGameObjectProxy, setRotation)
{
	static const GMString s_invoker = NAME ".setRotation";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".setRotation");
	GMGameObjectProxy self;
	GMVec4 v = GMArgumentHelper::popArgumentAsVec4(L, s_invoker).toVec4(); //quat
	GMQuat quat(v.getX(), v.getY(), v.getZ(), v.getW());
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->setRotation(quat);
	return GMReturnValues();
}


/*
* setTranslation([self], matrix)
*/
GM_LUA_PROXY_IMPL(GMGameObjectProxy, setScaling)
{
	static const GMString s_invoker = NAME ".setScaling";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".setScaling");
	GMGameObjectProxy self;
	GMMat4 mat = GMArgumentHelper::popArgumentAsMat4(L, s_invoker).toMat4(); //matrix
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->setScaling(mat);
	return GMReturnValues();
}

namespace
{
	// {{BEGIN META FUNCTION}}
	GM_LUA_FUNC(New)
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