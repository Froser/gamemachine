#include "stdafx.h"
#include "gmasset_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "gmmodel_meta.h"

#define NAME "GMAsset"

using namespace gm::luaapi;

GM_LUA_PROXY_GC_IMPL(GMAssetProxy, "GMAsset.__gc");

/*
 * getScene([self])
 */
GM_LUA_PROXY_IMPL(GMAssetProxy, getScene)
{
	static const GMString s_invoker = NAME ".getScene";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".getScene");
	GMAssetProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	GMSceneProxy scene = self->getScene();
	return GMReturnValues(L, scene);
}


/*
 * getModel([self])
 */
GM_LUA_PROXY_IMPL(GMAssetProxy, getModel)
{
	static const GMString s_invoker = "GMScene.getModel";
	GM_LUA_CHECK_ARG_COUNT(L, 1, "GMScene.getModel");
	GMAssetProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self

	auto m = self->getModel();
	GMModelProxy model(m);
	return GMReturnValues(L, model);
}

bool GMAssetProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META_FUNCTION(__gc);
	GM_META_FUNCTION(getScene);
	GM_META_FUNCTION(getModel);
	return true;
}

GMAssetProxy::GMAssetProxy(GMAsset asset)
{
	D(d);
	d->__handler = new GMAsset();
	*(d->__handler) = asset;
}

//////////////////////////////////////////////////////////////////////////
bool GMSceneProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META_FUNCTION(getModels);
	return true;
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