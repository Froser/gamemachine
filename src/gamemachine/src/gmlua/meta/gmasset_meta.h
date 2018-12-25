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
		GM_LUA_META_FUNCTION(__gc);
	};

	class GMAssetProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(GMAssetProxy)
		GM_LUA_META_PROXY_FUNCTIONS(GMAsset, asset)

	public:
		GMAssetProxy() = default;
		GMAssetProxy(GMAsset asset);

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif