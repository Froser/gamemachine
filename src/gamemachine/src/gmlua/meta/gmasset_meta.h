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

	class GMAssetProxy : public GMAnyProxy<GMAsset>
	{
		GM_LUA_PROXY_OBJ(GMAsset, GMAnyProxy)
		GM_DECLARE_PRIVATE_AND_BASE(GMAssetProxy, GMAnyProxy)

	public:
		GMAssetProxy(GMLuaCoreState* l, GMAsset asset);

	protected:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_OBJECT(GMSceneProxy)
	{
		GM_LUA_PROXY_FUNC(getModels);
	};

	class GMSceneProxy : public GMAnyProxy<GMScene>
	{
		GM_LUA_PROXY_OBJ(GMScene, GMAnyProxy)
		GM_DECLARE_PRIVATE_AND_BASE(GMSceneProxy, GMAnyProxy)

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif