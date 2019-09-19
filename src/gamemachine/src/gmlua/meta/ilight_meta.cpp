#include "stdafx.h"
#include "ilight_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "igamehandler_meta.h"

BEGIN_NS

#define NAME "ILight"

namespace luaapi
{
	GM_PRIVATE_OBJECT_UNALIGNED(ILightProxy)
	{
		GM_LUA_PROXY_FUNC(setLightAttribute3);
		GM_LUA_PROXY_FUNC(setLightAttribute);
	};

	/*
	 * setLightAttribute3([self], type, float3)
	 */
	GM_LUA_PROXY_IMPL(ILightProxy, setLightAttribute3)
	{
		GMLuaArguments args(L, NAME ".setLightAttribute3", { GMMetaMemberType::Object, GMMetaMemberType::Int, GMMetaMemberType::Vector3 } );
		ILightProxy self(L);
		args.getArgument(0, &self);
		GMint32 type = args.getArgument(1).toInt();
		GMVec3 v3 = args.getArgument(2).toVec3();
		self->setLightAttribute3(type, ValuePointer(v3));
		return GMReturnValues();
	}

	/*
	 * setLightAttribute([self], type, float)
	 */
	GM_LUA_PROXY_IMPL(ILightProxy, setLightAttribute)
	{
		GMLuaArguments args(L, NAME ".setLightAttribute", { GMMetaMemberType::Object, GMMetaMemberType::Int, GMMetaMemberType::Vector3 });
		ILightProxy self(L);
		args.getArgument(0, &self);
		GMint32 type = args.getArgument(1).toInt();
		GMfloat f = args.getArgument(2).toFloat();
		self->setLightAttribute(type, f);
		return GMReturnValues();
	}

	bool ILightProxy::registerMeta()
	{
		GM_META_FUNCTION(setLightAttribute3);
		GM_META_FUNCTION(setLightAttribute);
		return Base::registerMeta();
	}

	ILightProxy::ILightProxy(GMLuaCoreState* l, IDestroyObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}

	ILightProxy::~ILightProxy()
	{

	}

}
END_NS