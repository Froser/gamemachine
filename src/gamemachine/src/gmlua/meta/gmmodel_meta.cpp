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
	GM_META_FUNCTION(__index)
	return true;
}

GM_LUA_PROXY_IMPL(GMModelProxy, __index, L)
{
	GM_LUA_BEGIN_PROPERTY(GMModelProxy)
		GM_LUA_PROPERTY_PROXY_GETTER(GMShaderProxy, Shader, shader)
	GM_LUA_END_PROPERTY()
}