#ifndef __GM_LUA_ASSET_META_H__
#define __GM_LUA_ASSET_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gmassets.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_CLASS(GMAssetProxy);
	class GMAssetProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(GMAsset, GMAnyProxy)
		GM_DECLARE_PRIVATE(GMAssetProxy)
		GM_DECLARE_BASE(GMAnyProxy)

	public:
		GMAssetProxy(GMLuaCoreState* l, IDestroyObject* handler = nullptr);
		GMAssetProxy(GMLuaCoreState* l, GMAsset asset);
		GMAssetProxy(const GMAssetProxy& rhs);
		~GMAssetProxy();

	protected:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_OBJECT_UNALIGNED(GMSceneProxy)
	{
		GM_LUA_PROXY_FUNC(getModels);
	};

	GM_PRIVATE_CLASS(GMSceneProxy);
	class GMSceneProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(GMScene, GMAnyProxy)
		GM_DECLARE_PRIVATE(GMSceneProxy)
		GM_DECLARE_BASE(GMAnyProxy)

	public:
		GMSceneProxy(GMLuaCoreState* l, IDestroyObject* handler = nullptr);

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif