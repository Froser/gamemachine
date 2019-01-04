#include "stdafx.h"
#include "gmshader_meta.h"
#include <gamemachine.h>
#include <gmlua.h>

using namespace gm::luaapi;

/*
 * __index([self], key)
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(GMShaderProxy, __index, L)
{
	GM_LUA_BEGIN_PROPERTY_GETTER(GMShaderProxy)
		GM_LUA_PROPERTY_PROXY_GETTER(GMMaterialProxy, Material, material)
	GM_LUA_END_PROPERTY_GETTER()
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
	GM_META(shader);
	GM_META_FUNCTION(__index);
	return true;
}

//////////////////////////////////////////////////////////////////////////
/*
 * __index([self], key)
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(GMMaterialProxy, __index, L)
{
	GM_LUA_BEGIN_PROPERTY_GETTER(GMMaterialProxy)
		GM_LUA_PROPERTY_GETTER(Shininess, shininess)
		GM_LUA_PROPERTY_GETTER(Refractivity, refractivity)
		GM_LUA_PROPERTY_GETTER(Ambient, ambient)
		GM_LUA_PROPERTY_GETTER(Specular, specular)
		GM_LUA_PROPERTY_GETTER(Diffuse, diffuse)
		GM_LUA_PROPERTY_GETTER(F0, f0)
	GM_LUA_END_PROPERTY_GETTER()
}

bool GMMaterialProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META(__name);
	GM_META(material);
	GM_META_FUNCTION(__index);
	return true;
}

GMMaterialProxy::GMMaterialProxy(GMMaterial* material /*= nullptr*/)
{
	D(d);
	d->material = material;
}
