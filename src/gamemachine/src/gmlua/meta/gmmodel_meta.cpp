#include "stdafx.h"
#include "gmmodel_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "gmshader_meta.h"

using namespace gm::luaapi;

GMModelProxy::GMModelProxy(GMModel* model /*= nullptr*/)
{
	D(d);
	d->model = model;
}

bool GMModelProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META(model);
	GM_META(__name);
	GM_META_FUNCTION(getShader)
	return true;
}

/*
 * getShader([self])
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(GMModelProxy, getShader, L)
{
	static const GMString s_invoker = "GMModelProxy.getShader";
	GM_LUA_CHECK_ARG_COUNT(L, 1, "GMModelProxy.getShader");
	GMModelProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	GMShaderProxy shader(&self->getShader());
	return GMReturnValues(L, shader);
}