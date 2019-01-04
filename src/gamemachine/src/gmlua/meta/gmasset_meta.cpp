#include "stdafx.h"
#include "gmasset_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "gmmodel_meta.h"

#define NAME "GMAsset"

using namespace gm::luaapi;

/*
 * __gc([self])
 */
GM_LUA_PROXY_IMPL(GMAssetProxy, __gc, L)
{
	static const GMString s_invoker = NAME ".__gc";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".__gc");
	GMAssetProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	self.release();
	return GMReturnValues();
}

/*
 * getScene([self])
 */
GM_LUA_PROXY_IMPL(GMAssetProxy, getScene, L)
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
GM_LUA_PROXY_IMPL(GMAssetProxy, getModel, L)
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
	GM_META(asset);
	GM_META(__name);
	GM_META_FUNCTION(__gc);
	GM_META_FUNCTION(getScene);
	GM_META_FUNCTION(getModel);
	return true;
}

GMAssetProxy::GMAssetProxy(GMAsset asset)
{
	D(d);
	d->asset = new GMAsset();
	*(d->asset) = asset;
}

//////////////////////////////////////////////////////////////////////////
GMSceneProxy::GMSceneProxy(GMScene* scene)
{
	D(d);
	d->scene = scene;
}

bool GMSceneProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META(__name);
	GM_META(scene);
	GM_META_FUNCTION(getModels);
	return true;
}

/*
 * getModels([self])
 */
GM_LUA_PROXY_IMPL(GMSceneProxy, getModels, L)
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