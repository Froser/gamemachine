#include "stdafx.h"
#include "ilight_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "igamehandler_meta.h"

using namespace luaapi;

#define NAME "ILight"

/*
 * __gc([self])
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(ILightProxy, __gc, L)
{
	static const GMString s_invoker = NAME ".__gc";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".__gc");
	ILightProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	self.release();
	return GMReturnValues();
}

/*
 * setLightAttribute3([self], type, float3)
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(ILightProxy, setLightAttribute3, L)
{
	static const GMString s_invoker = NAME ".setLightAttribute3";
	GM_LUA_CHECK_ARG_COUNT(L, 3, NAME ".setLightAttribute3");
	ILightProxy self;
	GMVariant v3 = GMArgumentHelper::popArgumentAsVec3(L, s_invoker); //float3
	GMVariant type = GMArgumentHelper::popArgument(L, s_invoker); //type
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (v3.isVec3() && (type.isInt() || type.isInt64()))
		self->setLightAttribute3(type.toInt(), ValuePointer(v3.toVec3()));
	return GMReturnValues();
}

/*
 * setLightAttribute([self], type, float)
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(ILightProxy, setLightAttribute, L)
{
	static const GMString s_invoker = NAME ".setLightAttribute";
	GM_LUA_CHECK_ARG_COUNT(L, 3, NAME ".setLightAttribute");
	ILightProxy self;
	GMVariant f = GMArgumentHelper::popArgumentAsVec3(L, s_invoker); //float
	GMVariant type = GMArgumentHelper::popArgument(L, s_invoker); //type
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if ( (f.isFloat() || f.isInt()) && (type.isInt() || type.isInt64()) )
		self->setLightAttribute(type.toInt(), f.isFloat() ? f.toFloat() : f.toInt());
	return GMReturnValues();
}

ILightProxy::ILightProxy(ILight* light /*= nullptr*/)
{
	D(d);
	d->light = light;
}

bool ILightProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META(light);
	GM_META(__name);
	GM_META_FUNCTION(__gc);
	GM_META_FUNCTION(setLightAttribute3);
	GM_META_FUNCTION(setLightAttribute);
	return true;
}
