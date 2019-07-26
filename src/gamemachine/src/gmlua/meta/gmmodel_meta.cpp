#include "stdafx.h"
#include "gmmodel_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "gmshader_meta.h"

BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT_UNALIGNED(GMModelProxy)
	{
		GM_LUA_PROXY_FUNC(__index);
	};

	bool GMModelProxy::registerMeta()
	{
		GM_META_FUNCTION(__index)
			return Base::registerMeta();
	}

	GMModelProxy::GMModelProxy(GMLuaCoreState* l, IDestroyObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}

	GMModelProxy::~GMModelProxy()
	{

	}

	GM_LUA_PROXY_IMPL(GMModelProxy, __index)
	{
		GM_LUA_BEGIN_PROPERTY(GMModelProxy)
		GM_LUA_PROPERTY_PROXY_GETTER(GMShaderProxy, Shader, shader)
		GM_LUA_END_PROPERTY()
	}
}

END_NS