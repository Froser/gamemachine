#include "stdafx.h"
#include "gmshader_meta.h"
#include <gamemachine.h>
#include <gmlua.h>

BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT_UNALIGNED(GMShaderProxy)
	{
		GM_LUA_PROXY_FUNC(__index);
	};

	GM_PRIVATE_OBJECT_UNALIGNED(GMMaterialProxy)
	{
		GM_LUA_PROXY_FUNC(__index);
		GM_LUA_PROXY_FUNC(__newindex);
	};

	/*
	 * __index([self], key)
	 */
	GM_LUA_PROXY_IMPL(GMShaderProxy, __index)
	{
		GM_LUA_BEGIN_PROPERTY(GMShaderProxy)
		GM_LUA_PROPERTY_PROXY_GETTER(GMMaterialProxy, Material, material)
		GM_LUA_END_PROPERTY()
	}

	bool GMShaderProxy::registerMeta()
	{
		GM_META_FUNCTION(__index);
		return Base::registerMeta();
	}

	GMShaderProxy::GMShaderProxy(GMLuaCoreState* l, IDestroyObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}

	GMShaderProxy::~GMShaderProxy()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	/*
	 * __index([self], key)
	 */
	GM_LUA_PROXY_IMPL(GMMaterialProxy, __index)
	{
		GM_LUA_BEGIN_PROPERTY(GMMaterialProxy)
		GM_LUA_PROPERTY_GETTER(Shininess, shininess)
		GM_LUA_PROPERTY_GETTER(Refractivity, refractivity)
		GM_LUA_PROPERTY_GETTER(Ambient, ambient)
		GM_LUA_PROPERTY_GETTER(Specular, specular)
		GM_LUA_PROPERTY_GETTER(Diffuse, diffuse)
		GM_LUA_PROPERTY_GETTER(F0, f0)
		GM_LUA_END_PROPERTY()
	}

	/*
	 * __index([self], key)
	 */
	GM_LUA_PROXY_IMPL(GMMaterialProxy, __newindex)
	{
		GM_LUA_BEGIN_PROPERTY(GMMaterialProxy)

			indexMap["shininess"] = [](GMLuaCoreState* L, ProxyClass& m, const GMLuaArguments& args) {
				GMVariant value = args.getArgument(1);
				m->setSpecular(value.toVec3());
				return gm::GMReturnValues();
			};

			//GM_LUA_PROPERTY_SETTER(Shininess, shininess, GM_LUA_PROPERTY_TYPE_FLOAT)
			//GM_LUA_PROPERTY_SETTER(Refractivity, refractivity, GM_LUA_PROPERTY_TYPE_FLOAT)
			//GM_LUA_PROPERTY_SETTER(Ambient, ambient, GM_LUA_PROPERTY_TYPE_VEC3)
			//GM_LUA_PROPERTY_SETTER(Specular, specular, GM_LUA_PROPERTY_TYPE_VEC3)
			//GM_LUA_PROPERTY_SETTER(Diffuse, diffuse, GM_LUA_PROPERTY_TYPE_VEC3)
			//GM_LUA_PROPERTY_SETTER(F0, f0, GM_LUA_PROPERTY_TYPE_VEC3)
		GM_LUA_END_PROPERTY()
	}

	bool GMMaterialProxy::registerMeta()
	{
		GM_META_FUNCTION(__index);
		GM_META_FUNCTION(__newindex);
		return Base::registerMeta();
	}

	GMMaterialProxy::GMMaterialProxy(GMLuaCoreState* l, IDestroyObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}
}

END_NS