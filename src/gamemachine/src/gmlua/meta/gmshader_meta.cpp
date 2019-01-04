#include "stdafx.h"
#include "gmshader_meta.h"
#include <gamemachine.h>
#include <gmlua.h>

#define NAME "GMShader"

using namespace gm::luaapi;

/*
 * __index([self], key)
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(GMShaderProxy, __index, L)
{
	static const GMString s_invoker = NAME ".__index";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".__index");
	GMShaderProxy self;
	GMString key = GMArgumentHelper::popArgumentAsString(L, s_invoker);
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	return GMReturnValues();
}

/*
 * __newindex([self], key, value)
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(GMShaderProxy, __newindex, L)
{
	static const GMString s_invoker = NAME ".__newindex";
	GM_LUA_CHECK_ARG_COUNT(L, 3, NAME ".__newindex");
	GMShaderProxy self;
	//GMArgumentHelper::popArgumentAsObject(L, asset, s_invoker); //asset
	//GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	return GMReturnValues();
}

GMShaderProxy::GMShaderProxy(GMShader* shader /*= nullptr*/)
{
	D(d);
	d->shader = shader;
}

bool GMShaderProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META(__name);
	GM_META_FUNCTION(__index);
	GM_META_FUNCTION(__newindex);
	return true;
}
