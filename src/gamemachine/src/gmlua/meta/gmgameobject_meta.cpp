#include "stdafx.h"
#include "gmgameobject_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "gmasset_meta.h"

#define NAME "GMGameObject"

using namespace gm::luaapi;

bool GMGameObjectProxy::registerMeta()
{
	GM_META_FUNCTION(setAsset);
	GM_META_FUNCTION(setTranslation);
	GM_META_FUNCTION(setRotation);
	GM_META_FUNCTION(setScaling);
	GM_META_FUNCTION(update);
	return Base::registerMeta();
}

/*
 * setAsset([self], GMAsset)
 */
GM_LUA_PROXY_IMPL(GMGameObjectProxy, setAsset)
{
	static const GMString s_invoker = NAME ".setAsset";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".setAsset");
	GMGameObjectProxy self(L);
	GMAssetProxy asset(L);
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
	GMGameObjectProxy self(L);
	GMMat4 mat = GMArgumentHelper::popArgumentAsMat4(L, s_invoker).toMat4(); //matrix
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->setTranslation(mat);
	return GMReturnValues();
}

/*
 * setRotation([self], quat)
*/
GM_LUA_PROXY_IMPL(GMGameObjectProxy, setRotation)
{
	static const GMString s_invoker = NAME ".setRotation";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".setRotation");
	GMGameObjectProxy self(L);
	GMVec4 v = GMArgumentHelper::popArgumentAsVec4(L, s_invoker).toVec4(); //quat
	GMQuat quat(v.getX(), v.getY(), v.getZ(), v.getW());
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->setRotation(quat);
	return GMReturnValues();
}


/*
 * setScaling([self], matrix)
*/
GM_LUA_PROXY_IMPL(GMGameObjectProxy, setScaling)
{
	static const GMString s_invoker = NAME ".setScaling";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".setScaling");
	GMGameObjectProxy self(L);
	GMMat4 mat = GMArgumentHelper::popArgumentAsMat4(L, s_invoker).toMat4(); //matrix
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->setScaling(mat);
	return GMReturnValues();
}

/*
 * update([self], dt)
*/
GM_LUA_PROXY_IMPL(GMGameObjectProxy, update)
{
	static const GMString s_invoker = NAME ".update";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".update");
	GMGameObjectProxy self(L);
	GMfloat dt = GMArgumentHelper::popArgument(L, s_invoker).toFloat(); //dt
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->update(dt);
	return GMReturnValues();
}
namespace
{
	// {{BEGIN META FUNCTION}}
	GM_LUA_DEFAULT_NEW_IMPL(New, NAME ".new", GMGameObjectProxy, GMGameObject);
	// {{END META FUNCTION}}

	GMLuaReg g_meta[] = {
		// {{BEGIN META DECLARATIONS}}
		{ "new", New },
		// {{END META DECLARATIONS}}
		{ 0, 0 }
	};
}

GM_LUA_REGISTER_IMPL(GMGameObject_Meta, NAME, g_meta);