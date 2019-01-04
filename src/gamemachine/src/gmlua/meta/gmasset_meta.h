#ifndef __GM_LUA_ASSET_META_H__
#define __GM_LUA_ASSET_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gmassets.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMAssetProxy)
	{
		GM_LUA_PROXY;
		GMAsset* asset = nullptr;
		GMString __name = L"GMAsset";
		GM_LUA_PROXY_FUNC(__gc);
		GM_LUA_PROXY_FUNC(getScene);
		GM_LUA_PROXY_FUNC(getModel);
	};

	class GMAssetProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMAssetProxy, GMAsset, asset)

	public:
		GMAssetProxy() = default;
		GMAssetProxy(GMAsset asset);

	protected:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_OBJECT(GMSceneProxy)
	{
		GM_LUA_PROXY;
		GMScene* scene = nullptr;
		GMString __name = L"GMScene";
		GM_LUA_PROXY_FUNC(getModels);
	};

	class GMSceneProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMSceneProxy, GMScene, scene)

	public:
		GMSceneProxy() = default;
		GMSceneProxy(GMScene* scene);

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif