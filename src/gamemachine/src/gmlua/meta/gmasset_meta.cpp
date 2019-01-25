#include "stdafx.h"
#include "gmasset_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "gmmodel_meta.h"

#define NAME "GMAsset"

using namespace gm::luaapi;

/*
 * getScene([self])
 */
GM_LUA_PROXY_IMPL(GMAssetProxy, getScene)
{
	static const GMString s_invoker = NAME ".getScene";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".getScene");
	GMAssetProxy self(L);
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
	{
		GMSceneProxy scene = self->getScene();
		return GMReturnValues(L, scene);
	}
	return GMReturnValues();
}

/*
 * getModel([self])
 */
GM_LUA_PROXY_IMPL(GMAssetProxy, getModel)
{
	static const GMString s_invoker = "GMScene.getModel";
	GM_LUA_CHECK_ARG_COUNT(L, 1, "GMScene.getModel");
	GMAssetProxy self(L);
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
	{
		auto m = self->getModel();
		GMModelProxy model(L, m);
		return GMReturnValues(L, model);
	}
	return GMReturnValues();
}

bool GMAssetProxy::registerMeta()
{
	GM_META_FUNCTION(getScene);
	GM_META_FUNCTION(getModel);
	return Base::registerMeta();
}

GMAssetProxy::GMAssetProxy(GMAsset asset)
{
	D_BASE(d, Base);
	d->__handler = new GMAsset();
	*(d->__handler) = asset;
}

//////////////////////////////////////////////////////////////////////////
bool GMSceneProxy::registerMeta()
{
	GM_META_FUNCTION(getModels);
	return Base::registerMeta();
}

/*
 * getModels([self])
 */
GM_LUA_PROXY_IMPL(GMSceneProxy, getModels)
{
	static const GMString s_invoker = "GMScene.getModels";
	GM_LUA_CHECK_ARG_COUNT(L, 1, "GMScene.getModels");
	GMSceneProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self

	auto& ms = self->getModels();
	Vector<GMAssetProxy>* assets = new Vector<GMAssetProxy>();
	for (auto model : ms)
	{
		assets->emplace_back(model);
	}

	GMLuaVector<GMAssetProxy> models(assets);
	return GMReturnValues(L, models);
}