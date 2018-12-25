#include "stdafx.h"
#include "gmasset_meta.h"
#include <gamemachine.h>
#include <gmlua.h>

#define NAME "GMAsset"

using namespace gm::luaapi;

/*
 * __gc([self])
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(GMAssetProxy, __gc, L)
{
	static const GMString s_invoker = NAME ".__gc";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".__gc");
	GMAssetProxy asset;
	GMArgumentHelper::popArgumentAsObject(L, asset, s_invoker); //self
	asset.release();
	return GMReturnValues();
}

bool GMAssetProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META(asset);
	GM_META(__name);
	GM_META_FUNCTION(__gc)
	return true;
}

GMAssetProxy::GMAssetProxy(GMAsset asset)
{
	D(d);
	d->asset = new GMAsset();
	*(d->asset) = asset;
}
