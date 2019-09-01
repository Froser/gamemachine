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
		static const GMString s_invoker = NAME ".setLightAttribute3";
		GM_LUA_CHECK_ARG_COUNT(L, 3, NAME ".setLightAttribute3");
		ILightProxy self(L);
		GMVariant v3 = GMArgumentHelper::popArgumentAsVec3(L, s_invoker); //float3
		GMVariant type = GMArgumentHelper::popArgument(L, s_invoker); //type
		GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
		if (v3.isVec3() && (type.isInt() || type.isInt64()))
			self->setLightAttribute3(type.toInt(), ValuePointer(v3.toVec3()));
		return gm::GMReturnValues();
	}

	/*
	 * setLightAttribute([self], type, float)
	 */
	GM_LUA_PROXY_IMPL(ILightProxy, setLightAttribute)
	{
		static const GMString s_invoker = NAME ".setLightAttribute";
		GM_LUA_CHECK_ARG_COUNT(L, 3, NAME ".setLightAttribute");
		ILightProxy self(L);
		GMVariant f = GMArgumentHelper::popArgumentAsVec3(L, s_invoker); //float
		GMVariant type = GMArgumentHelper::popArgument(L, s_invoker); //type
		GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
		if ((f.isFloat() || f.isInt()) && (type.isInt() || type.isInt64()))
			self->setLightAttribute(type.toInt(), f.isFloat() ? f.toFloat() : f.toInt());
		return gm::GMReturnValues();
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