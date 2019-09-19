#include "stdafx.h"
#include "gmgameobject_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "gmasset_meta.h"

#define NAME "GMGameObject"

BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT_UNALIGNED(GMGameObjectProxy)
	{
		GM_LUA_PROXY_FUNC(setAsset);
		GM_LUA_PROXY_FUNC(setTranslation);
		GM_LUA_PROXY_FUNC(setRotation);
		GM_LUA_PROXY_FUNC(setScaling);
		GM_LUA_PROXY_FUNC(update);
	};

	bool GMGameObjectProxy::registerMeta()
	{
		GM_META_FUNCTION(setAsset);
		GM_META_FUNCTION(setTranslation);
		GM_META_FUNCTION(setRotation);
		GM_META_FUNCTION(setScaling);
		GM_META_FUNCTION(update);
		return Base::registerMeta();
	}

	GMGameObjectProxy::GMGameObjectProxy(GMLuaCoreState* l, GMObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}

	GMGameObjectProxy::~GMGameObjectProxy()
	{

	}

	/*
	 * setAsset([self], GMAsset)
	 */
	GM_LUA_PROXY_IMPL(GMGameObjectProxy, setAsset)
	{
		static const GMString s_invoker = NAME ".setAsset";
		GMLuaArguments args(L, NAME ".setAsset", { GMMetaMemberType::Object, GMMetaMemberType::Object });
		GMGameObjectProxy self(L);
		GMAssetProxy asset(L);
		args.getArgument(0, &self);
		args.getArgument(1, &asset);
		if (self)
			self->setAsset(*asset.get());
		return GMReturnValues();
	}

	/*
	 * setTranslation([self], matrix)
	 */
	GM_LUA_PROXY_IMPL(GMGameObjectProxy, setTranslation)
	{
		GMLuaArguments args(L, NAME ".setTranslation", { GMMetaMemberType::Object, GMMetaMemberType::Matrix4x4 });
		GMGameObjectProxy self(L);
		args.getArgument(0, &self);
		GMMat4 mat = args.getArgument(1).toMat4();
		if (self)
			self->setTranslation(mat);
		return GMReturnValues();
	}

	/*
	 * setRotation([self], quat)
	*/
	GM_LUA_PROXY_IMPL(GMGameObjectProxy, setRotation)
	{
		GMLuaArguments args(L, NAME ".setRotation", { GMMetaMemberType::Object, GMMetaMemberType::Vector4 });
		GMGameObjectProxy self(L);
		args.getArgument(0, &self);
		GMVec4 v = args.getArgument(1).toVec4();
		GMQuat quat(v.getX(), v.getY(), v.getZ(), v.getW());
		if (self)
			self->setRotation(quat);
		return GMReturnValues();
	}


	/*
	 * setScaling([self], matrix)
	*/
	GM_LUA_PROXY_IMPL(GMGameObjectProxy, setScaling)
	{
		GMLuaArguments args(L, NAME ".setScaling", { GMMetaMemberType::Object, GMMetaMemberType::Matrix4x4 });
		GMGameObjectProxy self(L);
		args.getArgument(0, &self);
		GMMat4 mat = args.getArgument(1).toMat4();
		if (self)
			self->setScaling(mat);
		return GMReturnValues();
	}

	/*
	 * update([self], dt)
	*/
	GM_LUA_PROXY_IMPL(GMGameObjectProxy, update)
	{
		GMLuaArguments args(L, NAME ".update", { GMMetaMemberType::Object, GMMetaMemberType::Float });
		GMGameObjectProxy self(L);
		args.getArgument(0, &self);
		GMfloat dt = args.getArgument(1).toFloat();
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
}

END_NS