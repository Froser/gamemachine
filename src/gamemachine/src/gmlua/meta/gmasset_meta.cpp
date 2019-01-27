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
		GMSceneProxy scene(L);
		scene.set(self->getScene());
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
		GMModelProxy model(L);
		model.set(m);
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

GMAssetProxy::GMAssetProxy(GMLuaCoreState* l, GMAsset asset)
	: Base(l)
{
	D_BASE(d, Base);
	d->__handler = new GMAsset();
	// __handler不是GMAsset类型，因此会调用基类(IDestroyObject)拷贝构造函数
	// 因此需要强行转换，否则会造成拷贝切割
	*static_cast<GMAsset*>(d->__handler) = asset;
}

GMAssetProxy::GMAssetProxy(const GMAssetProxy& rhs)
	: GMAnyProxy(rhs.getLuaCoreState(), rhs.get())
{
}

GMAssetProxy::~GMAssetProxy()
{
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
	GMSceneProxy self(L);
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self

	auto& ms = self->getModels();
	Vector<GMAssetProxy>* assets = new Vector<GMAssetProxy>();
	for (auto model : ms)
	{
		assets->emplace_back(L, model);
	}

	GMLuaVector<GMAssetProxy> models(assets);
	return GMReturnValues(L, models);
}