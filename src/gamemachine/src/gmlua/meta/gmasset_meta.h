#ifndef __GM_LUA_ASSET_META_H__
#define __GM_LUA_ASSET_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gmassets.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMAssetProxy)
	{
		GM_LUA_PROXY_FUNC(getScene);
		GM_LUA_PROXY_FUNC(getModel);
	};

	class GMAssetProxy : public GMObjectProxy
	{
		GM_LUA_PROXY_OBJ(GMAssetProxy, GMAsset, GMObjectProxy)
		GM_DECLARE_PRIVATE(GMAssetProxy)

	public:
		GMAssetProxy() = default;
		GMAssetProxy(GMAsset asset);

	protected:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_OBJECT(GMSceneProxy)
	{
		GM_LUA_PROXY(GMScene);
		GM_LUA_PROXY_FUNC(getModels);
	};

	class GMSceneProxy : public GMObjectProxy
	{
		GM_LUA_PROXY_OBJ(GMSceneProxy, GMScene, GMObjectProxy)
		GM_DECLARE_PRIVATE(GMSceneProxy)

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif