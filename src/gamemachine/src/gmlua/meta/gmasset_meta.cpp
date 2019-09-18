#include "stdafx.h"
#include "gmasset_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "gmmodel_meta.h"


BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT_UNALIGNED(GMAssetProxy)
	{
		GM_LUA_PROXY_FUNC(getScene);
		GM_LUA_PROXY_FUNC(getModel);
	};

#define NAME "GMAsset"
	/*
	 * getScene([self])
	 */
	GM_LUA_PROXY_IMPL(GMAssetProxy, getScene)
	{
		GMLuaArguments args(L, NAME ".getScene", { GMMetaMemberType::Object });
		GMAssetProxy self(L);
		args.getArgument(0, &self);
		if (self)
		{
			GMSceneProxy scene(L);
			scene.set(self->getScene());
			return gm::GMReturnValues(L, scene);
		}
		return gm::GMReturnValues();
	}

	/*
	 * getModel([self])
	 */
	GM_LUA_PROXY_IMPL(GMAssetProxy, getModel)
	{
		GMLuaArguments args(L, NAME ".getModel", { GMMetaMemberType::Object });
		GMAssetProxy self(L);
		args.getArgument(0, &self);
		if (self)
		{
			auto m = self->getModel();
			GMModelProxy model(L);
			model.set(m);
			return gm::GMReturnValues(L, model);
		}
		return gm::GMReturnValues();
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
		GM_CREATE_DATA();
		D_BASE(d, Base);
		d->__handler = new GMAsset();
		// __handler不是GMAsset类型，因此会调用基类(IDestroyObject)拷贝构造函数
		// 因此需要强行转换，否则会造成拷贝切割
		*static_cast<GMAsset*>(d->__handler) = asset;
	}

	GMAssetProxy::GMAssetProxy(const GMAssetProxy& rhs)
		: GMAnyProxy(rhs.getLuaCoreState(), rhs.get())
	{
		GM_CREATE_DATA();
	}

	GMAssetProxy::GMAssetProxy(GMLuaCoreState* l, IDestroyObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}

	GMAssetProxy::~GMAssetProxy()
	{
	}

	//////////////////////////////////////////////////////////////////////////
#undef NAME
#define NAME "GMScene"
	bool GMSceneProxy::registerMeta()
	{
		GM_META_FUNCTION(getModels);
		return Base::registerMeta();
	}

	GMSceneProxy::GMSceneProxy(GMLuaCoreState* l, IDestroyObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}

	/*
	 * getModels([self])
	 */
	GM_LUA_PROXY_IMPL(GMSceneProxy, getModels)
	{
		GMLuaArguments args(L, NAME ".getModels", { GMMetaMemberType::Object });
		GMSceneProxy self(L);
		args.getArgument(0, &self);

		auto& ms = self->getModels();
		Vector<GMAssetProxy>* assets = new Vector<GMAssetProxy>();
		for (auto model : ms)
		{
			assets->emplace_back(L, model);
		}

		GMLuaVector<GMAssetProxy> models(assets);
		return gm::GMReturnValues(L, models);
	}
}
END_NS